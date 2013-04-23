/*
 * AbstractCoreBoundQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#ifndef ABSTRACTCOREBOUNDQUEUESSCHEDULER_H_
#define ABSTRACTCOREBOUNDQUEUESSCHEDULER_H_

#include "AbstractTaskScheduler.h"
#include "AbstractCoreBoundQueue.h"
#include <atomic>

class AbstractCoreBoundQueuesScheduler : public AbstractTaskScheduler, public TaskReadyObserver{

 public:

  typedef AbstractCoreBoundQueue task_queue_t;
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
  typedef std::vector<task_queue_t *> task_queues_t;

 protected:

  // set for tasks with open dependencies
  waiting_tasks_t _waitSet;
  // task queues to dispatch tasks to
  task_queues_t _taskQueues;
  // number of queues
  size_t _queues;
  // scheduler status
  std::atomic<scheduler_status_t> _status;
  // mutex to protect waitset
  std::mutex _setMutex;
  // mutex to protect task queues
  std::mutex _queuesMutex;
  // holds the queue that gets the next task (simple roundrobin, first)
  std::atomic<size_t> _nextQueue;

  static log4cxx::LoggerPtr _logger;

  /**
   * push ready task to the next queue
   */
  virtual void pushToQueue(std::shared_ptr<Task> task) = 0;

  /*
   * stop a specific queue and redistribute tasks to other queues
   */
  virtual void stopQueueAndRedistributeTasks(task_queue_t *queue, int queues);
  /*
   * create a new task queue
   */
  virtual task_queue_t *createTaskQueue(int core) = 0;

 public:
  AbstractCoreBoundQueuesScheduler();

  virtual ~AbstractCoreBoundQueuesScheduler();
  /*
   * return scheduler status
   */
  scheduler_status_t getSchedulerStatus();
  /*
   * schedule a given task
   */
  virtual void schedule(std::shared_ptr<Task> task);
  /*
   * schedule a task for execution on a given core
   *
   */
  void schedule(std::shared_ptr<Task> task, int core);

  /* change the number of threads the task scheduler uses for running tasks;
   *
   */
  virtual void resize(const size_t queues);
  /*
   * notify scheduler that a given task is ready
   */
  void notifyReady(std::shared_ptr<Task> task);

  /*
   * waits for all tasks to finish
   */
  void wait();

  size_t getNumberOfWorker() const;

  virtual void shutdown();

};

#endif /* ABSTRACTCOREBOUNDQUEUESSCHEDULER_H_ */
