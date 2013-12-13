/*
 * CentralPriorityScheduler.h
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#pragma once

#include "AbstractTaskScheduler.h"
#include "helper/HwlocHelper.h"
#include <memory>
#include <thread>
#include <queue>
#include <vector>
#include <condition_variable>

namespace hyrise {
namespace taskscheduler {

class CentralPriorityScheduler;

// our worker thread objects
class PriorityWorkerThread {
private:
    CentralPriorityScheduler &scheduler;
public:

  typedef AbstractTaskScheduler::lock_t lock_t;

    PriorityWorkerThread(CentralPriorityScheduler &s) : scheduler(s) { }
    void operator()();
};


/**
 * a central scheduler holds a task queue and n worker threads
 */
class CentralPriorityScheduler : 
  public AbstractTaskScheduler,
  public TaskReadyObserver,
  public std::enable_shared_from_this<TaskReadyObserver> {
  friend class PriorityWorkerThread;
protected:
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
  // set for tasks with open dependencies
  waiting_tasks_t _waitSet;
  // mutex to protect waitset
  lock_t _setMutex;
  // queue of tasks that are ready to run
  std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>, CompareTaskPtr> _runQueue;
  // mutex to protect ready queue
  lock_t _queueMutex;
  // vector of worker threads
  std::vector<std::thread> _worker_threads;
  // condition variable to wake up workers
  std::condition_variable_any _condition;
  // scheduler status
  scheduler_status_t _status;
  // mutex to protect status
  lock_t _statusMutex;

  static log4cxx::LoggerPtr _logger;


public:
  CentralPriorityScheduler(int threads = getNumberOfCoresOnSystem());
  virtual ~CentralPriorityScheduler();

  void worker_loop();
  /*
   * schedule a task for execution
   */
  virtual void schedule(std::shared_ptr<Task> task);
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  void shutdown();
  /**
   * get number of worker
   */
  size_t getNumberOfWorker() const;

  virtual void notifyReady(std::shared_ptr<Task> task);

};

} } // namespace hyrise::taskscheduler

