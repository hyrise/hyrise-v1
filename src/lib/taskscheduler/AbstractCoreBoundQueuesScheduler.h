/*
 * AbstractCoreBoundQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "AbstractTaskScheduler.h"
#include "AbstractCoreBoundQueue.h"
#include <atomic>

namespace hyrise {
namespace taskscheduler {

class AbstractCoreBoundQueuesScheduler : 
  public AbstractTaskScheduler,
  public TaskReadyObserver,
  public std::enable_shared_from_this<TaskReadyObserver> {

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
  lock_t _setMutex;
  // mutex to protect task queues
  lock_t _queuesMutex;
  // holds the queue that gets the next task (simple roundrobin, first)
  size_t _nextQueue;

  static log4cxx::LoggerPtr _logger;

  /**
   * push ready task to the next queue
   */
  virtual void pushToQueue(std::shared_ptr<Task> task) = 0;

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

} } // namespace hyrise::taskscheduler

