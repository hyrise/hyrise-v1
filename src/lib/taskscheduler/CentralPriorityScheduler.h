/*
 * CentralPriorityScheduler.h
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#ifndef CENTRALPRIORITYSCHEDULER_H_
#define CENTRALPRIORITYSCHEDULER_H_

#include "AbstractTaskScheduler.h"
#include "helper/HwlocHelper.h"
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <condition_variable>
#include <taskscheduler/SharedScheduler.h>

class CentralPriorityScheduler;

// our worker thread objects
class PriorityWorkerThread {
private:
    CentralPriorityScheduler &scheduler;
public:
    PriorityWorkerThread(CentralPriorityScheduler &s) : scheduler(s) { }
    void operator()();
};


/**
 * a central scheduler holds a task queue and n worker threads
 */
class CentralPriorityScheduler : public AbstractTaskScheduler, public TaskReadyObserver {
  friend class PriorityWorkerThread;
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
  // set for tasks with open dependencies
  waiting_tasks_t _waitSet;
  // mutex to protect waitset
  std::mutex _setMutex;
  // queue of tasks that are ready to run
  std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>, CompareTaskPtr> _runQueue;
  // mutex to protect ready queue
  std::mutex _queueMutex;
  // vector of worker threads
  std::vector<std::thread *> _worker_threads;
  // condition variable to wake up workers
  std::condition_variable _condition;
  // scheduler status
  scheduler_status_t _status;
  // mutex to protect status
  std::mutex _statusMutex;

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

#endif /* CENTRALPRIORITYSCHEDULER_H_ */
