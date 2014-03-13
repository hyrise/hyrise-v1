// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <atomic>
#include "AbstractTaskScheduler.h"
#include "ThreadLevelQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class ThreadLevelQueuesScheduler;
typedef ThreadLevelQueuesScheduler<PriorityQueueType> ThreadLevelPriorityQueuesScheduler;
typedef ThreadLevelQueuesScheduler<BasicQueueType> ThreadLevelBasicQueuesScheduler;
/*
* 2-Level-Scheduler: This scheduler dispatches tasks to queues,
* each running a thread for task execution
*/
template <class QUEUE>
class ThreadLevelQueuesScheduler : public AbstractTaskScheduler,
                                   public TaskReadyObserver,
                                   public std::enable_shared_from_this<TaskReadyObserver> {

 public:
  typedef ThreadLevelQueue<QUEUE> task_queue_t;
  typedef std::vector<std::shared_ptr<task_queue_t>> task_queues_t;

 protected:
  size_t _queueCount;
  std::atomic<size_t> _nextQueue;
  task_queues_t _queues;
  scheduler_status_t _status;
  static log4cxx::LoggerPtr _logger;

  virtual size_t getNextQueue() {
    _nextQueue = (_nextQueue + 1) % _queueCount;
    return _nextQueue;
  }

  virtual void pushToQueue(const std::shared_ptr<Task>& task) {
    size_t q = getNextQueue();
    // simple strategy to avoid blocking of queues; check if queue is blocked - try a couple of times, otherwise
    // schedule on next queue
    size_t retries = 0;
    while (_queues[q]->blocked() && retries < 100) {
      q = getNextQueue();
      ++retries;
    }
    _queues[q]->schedule(task);
  }

  virtual std::shared_ptr<task_queue_t> createTaskQueue(int core) {
    return std::make_shared<ThreadLevelQueue<QUEUE>>(1);
  }

 public:
  ThreadLevelQueuesScheduler(int queues) : _queueCount(queues), _nextQueue(0), _status(START_UP) {};
  ~ThreadLevelQueuesScheduler() { shutdown(); }

  /*
   * init task scheduler
   */
  virtual void init() {
    for (size_t i = 0; i < _queueCount; i++) {
      _queues.push_back(createTaskQueue(i));
      _queues[i]->init();
    }
    _status = RUN;
  }
  /*
   * schedule a task for execution
   */
  virtual void schedule(const std::shared_ptr<Task>& task) {
    task->lockForNotifications();
    if (task->isReady()) {
      task->unlockForNotifications();
      pushToQueue(task);
    } else {
      task->addReadyObserver(shared_from_this());
    }
    task->unlockForNotifications();
  }
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown() {
    if (_status != STOPPED) {
      for (size_t i = 0; i < _queueCount; i++)
        _queues[i]->shutdown();
      _status = STOPPED;
    }
  }
  /**
   * get number of worker
   */
  virtual size_t getNumberOfWorker() const { return _queueCount; }

  virtual void notifyReady(const std::shared_ptr<Task>& task) { pushToQueue(task); }
};

template <class QUEUE>
log4cxx::LoggerPtr ThreadLevelQueuesScheduler<QUEUE>::_logger =
    log4cxx::Logger::getLogger("taskscheduler.ThreadLevelQueuesScheduler");
}
}
