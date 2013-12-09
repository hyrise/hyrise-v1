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
  TaskExecutor(std::shared_ptr<Task> task) : _task(task) { }
  void operator()(){
    (*_task)();
    _task->notifyDoneObservers();
  };
};

class ThreadPerTaskScheduler : 
  public AbstractTaskScheduler,
  public TaskReadyObserver,
  public std::enable_shared_from_this<TaskReadyObserver> {
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
    // set for tasks with open dependencies
    waiting_tasks_t _waitSet;
    // mutex to protect waitset
    lock_t _setMutex;
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
  virtual void schedule(std::shared_ptr<Task> task);
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown();

  /**
   * get number of worker
   */
  virtual size_t getNumberOfWorker() const {
      return 0;
  };

  void notifyReady(std::shared_ptr<Task> task);
};

} } // namespace hyrise::taskscheduler

