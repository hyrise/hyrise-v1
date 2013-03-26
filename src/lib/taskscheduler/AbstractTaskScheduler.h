// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * AbstractTaskScheduler.h
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_ABSTRACTTASKSCHEDULER_H_
#define SRC_LIB_TASKSCHEDULER_ABSTRACTTASKSCHEDULER_H_

/*
 * Base class for Task Schedulers
 *
 */

#include "taskscheduler/Task.h"
#include "taskscheduler/AbstractTaskQueue.h"
#include <memory>
#include <unordered_set>
#include <iostream>
#include <log4cxx/logger.h>
#include "helper/HwlocHelper.h"

class AbstractTaskScheduler {

 public:
  virtual ~AbstractTaskScheduler() {};
  /*
   * schedule a task for execution
   */
  virtual void schedule(std::shared_ptr<Task> task) = 0;
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown() = 0;
  /*
   * resize the number of worker threads/queues
   */
  virtual void resize(const size_t queues) = 0;
  /**
   * get number of worker
   */
  virtual size_t getNumberOfWorker() const = 0;
};

template <class TaskQueue>
class AbstractQueueBasedTaskScheduler : public AbstractTaskScheduler, public TaskReadyObserver{

  /*
   * definition of queue status, can be used to sync actions in queue, like work stealing, with scheduler status (e.g. to avoid stealing tasks from queue, while resizing)
   */
 public:
  typedef enum {
    START_UP = -1,
    RUN = 0,
    RESIZING = 1,
    TO_STOP = 2,
    STOPPED = 3
  } scheduler_status_t;

 protected:
  typedef TaskQueue task_queue_t;
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
  typedef std::vector<task_queue_t *> task_queues_t;
  // set for tasks with open dependencies
  waiting_tasks_t _waitSet;
  // task queues to dispatch tasks to
  task_queues_t _taskQueues;
  // number of queues
  size_t _queues;
  // scheduler status
  scheduler_status_t _status;
  // mutex to protect waitset
  std::mutex _setMutex;
  // mutex to protect status
  std::mutex _statusMutex;
  // mutex to protect task queues
  std::mutex _queuesMutex;
  // holds the queue that gets the next task (simple roundrobin, first)
  size_t _nextQueue;

  static log4cxx::LoggerPtr _logger;

  AbstractQueueBasedTaskScheduler(): _queues(0), _status(START_UP), _nextQueue(0) {
  };

  /**
   * push ready task to the next queue
   */
  virtual void pushToQueue(std::shared_ptr<Task> task) = 0;

  /*
   * stop a specific queue and redistribute tasks to other queues
   */
  virtual void stopQueueAndRedistributeTasks(task_queue_t *queue, int queues) = 0;
  /*
   * create a new task queue
   */
  virtual task_queue_t *createTaskQueue(int core) = 0;

 public:

  virtual ~AbstractQueueBasedTaskScheduler() {};
  /*
   * return scheduler status
   */
  scheduler_status_t getSchedulerStatus() {
    std::lock_guard<std::mutex> lk2(_statusMutex);
    return _status;
  }
  /*
   * schedule a given task
   */
  virtual void schedule(std::shared_ptr<Task> task) {
    // simple strategy: check if task is ready to run -> then move to next taskqueue
    // otherwise store in wait list

    // lock the task - otherwise, a notify might happen prior to the task being added to the wait set
    task->lockForNotifications();
    if (task->isReady())
      pushToQueue(task);
    else {
      task->addReadyObserver(this);
      std::lock_guard<std::mutex> lk(_setMutex);
      _waitSet.insert(task);
      LOG4CXX_DEBUG(_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " inserted in wait queue");
    }
    task->unlockForNotifications();
  }
  /*
   * schedule a task for execution on a given core
   *
   */
  void schedule(std::shared_ptr<Task> task, int core) {
    task->setPreferredCore(core);
    schedule(task);
  }

  /* change the number of threads the task scheduler uses for running tasks;
   *
   */
  void resize(const size_t queues) {
    // set status to RESIZING
    // lock scheduler
    _statusMutex.lock();
    _status = RESIZING;
    _statusMutex.unlock();

    if (queues > _queues) {
      //set _queues to queues after new queues have been created to new tasks to be assigned to new queues
      // lock _queue mutex as queues are manipulated
      std::lock_guard<std::mutex> lk(_queuesMutex);
      if (static_cast<int>(queues) <= getNumberOfCoresOnSystem()) {
        for (size_t i = _queues; i < queues; ++i) {
          _taskQueues.push_back(createTaskQueue(i));
        }
        _queues = queues;
      } else {
        LOG4CXX_WARN(_logger, "number of queues exceeds available cores; set it to max available cores, which equals to " << std::to_string(getNumberOfCoresOnSystem()));
        resize(getNumberOfCoresOnSystem());
      }
    } else if (queues < _queues) {
      //set _queues to queues before queues have been deleted to avoid new tasks to be assigned to old queues
      // lock _queue mutex as queues are manipulated
      size_t queues_old;
      {
        std::lock_guard<std::mutex> lk(_queuesMutex);
        queues_old = _queues;
        _queues = queues;
        //adjust nextQueue to assign a task to in case it is larger than queue size;
        if (!(_nextQueue < queues))
          _nextQueue = 0;
      }
      for (size_t i = queues_old - 1; i > queues - 1; --i) {
        stopQueueAndRedistributeTasks(_taskQueues[i], queues);
        {
          std::lock_guard<std::mutex> lk(_queuesMutex);
          delete _taskQueues.back();
          _taskQueues.pop_back();
        }
      }
    }
    _statusMutex.lock();
    _status = RUN;
    _statusMutex.unlock();
  }
  /*
   * notify scheduler that a given task is ready
   */
  virtual void notifyReady(std::shared_ptr<Task> task) {
    // remove task from wait set
    _setMutex.lock();
    int tmp = _waitSet.erase(task);
    _setMutex.unlock();

    // if task was found in wait set, schedule task to next queue
    if (tmp == 1) {
      LOG4CXX_DEBUG(_logger, "Task " << std::hex << (void *)task.get() << std::dec << " ready to run");
      pushToQueue(task);
    } else
      // should never happen, but check to identify potential race conditions
      LOG4CXX_ERROR(_logger, "Task that notified to be ready to run was not found / found more than once in waitSet! " << std::to_string(tmp));
  }

  /*
   * waits for all tasks to finish
   */
  void wait() {
    for (unsigned i = 0; i < _taskQueues.size(); ++i) {
      _taskQueues[i]->join();
    }
  }

  size_t getNumberOfWorker() const {
    return _queues;
  }

  virtual void shutdown() {
    _statusMutex.lock();
    _status = TO_STOP;
    _statusMutex.unlock();
    for (unsigned i = 0; i < _taskQueues.size(); ++i) {
      _taskQueues[i]->stopQueue();
    }
    _statusMutex.lock();
    _status = STOPPED;
    _statusMutex.unlock();
  }
};

template <class TaskQueue> log4cxx::LoggerPtr AbstractQueueBasedTaskScheduler<TaskQueue>::_logger = log4cxx::Logger::getLogger("taskscheduler.AbstractQueueBasedTaskScheduler");

#endif  // SRC_LIB_TASKSCHEDULER_ABSTRACTTASKSCHEDULER_H_
