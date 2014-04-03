// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

/*
 * ThreadPerTaskScheduler.h
 *
 *  Created on: Jul 22, 2013
 *      Author: jwust
 */

#pragma once

#include "AbstractTaskScheduler.h"
#include <taskscheduler/SharedScheduler.h>
#include <memory>

#include <thread>
#include <queue>

namespace hyrise {
namespace taskscheduler {

// our worker thread objects
class TaskExecutor {
  std::shared_ptr<Task> _task;

 public:
  TaskExecutor(std::shared_ptr<Task> task) : _task(task) {}
  void operator()() {
    (*_task)();
    _task->notifyDoneObservers();
  };
};
/*
* A scheduler that starts a new thread for each arriving task
*/
class ThreadPerTaskScheduler : public AbstractTaskScheduler,
                               public TaskReadyObserver,
                               public std::enable_shared_from_this<TaskReadyObserver> {
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
  // scheduler status
  scheduler_status_t _status;
  // mutex to protect status
  lock_t _statusMutex;
  static log4cxx::LoggerPtr _logger;

 public:
  ThreadPerTaskScheduler();
  ThreadPerTaskScheduler(int i);
  virtual ~ThreadPerTaskScheduler();
  /*
   * schedule a task for execution
   */
  virtual void schedule(const std::shared_ptr<Task>& task);
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown();

  virtual void init();
  /**
   * get number of worker
   */
  virtual size_t getNumberOfWorker() const {
    return 0;
  };

  void notifyReady(const std::shared_ptr<Task>& task);
};
}
}  // namespace hyrise::taskscheduler
