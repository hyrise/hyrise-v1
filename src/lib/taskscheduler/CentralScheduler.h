/*
 * CentralScheduler.h
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
#include <condition_variable>
#include <taskscheduler/SharedScheduler.h>
#include "tbb/concurrent_queue.h"

namespace hyrise {
namespace taskscheduler {

class CentralScheduler;

// our worker thread objects
class WorkerThread {
 private:
  CentralScheduler& scheduler;

 public:
  typedef AbstractTaskScheduler::lock_t lock_t;

  WorkerThread(CentralScheduler& s) : scheduler(s) {}
  void operator()();
};


/**
 * a central scheduler holds a task queue and n worker threads
 */
class CentralScheduler : public AbstractTaskScheduler,
                         public TaskReadyObserver,
                         public std::enable_shared_from_this<TaskReadyObserver> {
  friend class WorkerThread;

 protected:
  // queue of tasks that are ready to run
  tbb::concurrent_queue<std::shared_ptr<Task>> _runQueue;
  std::vector<std::thread> _worker_threads;
  // scheduler status
  scheduler_status_t _status;
  static log4cxx::LoggerPtr _logger;
  // number of threads
  int _threads;

 public:
  CentralScheduler(int threads = getNumberOfCoresOnSystem());
  virtual ~CentralScheduler();
  /*
   * init task scheduler
   */
  virtual void init();

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
}
}  // namespace hyrise::taskscheduler
