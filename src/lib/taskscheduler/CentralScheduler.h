/*
 * CentralScheduler.h
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#ifndef CENTRALSCHEDULER_H_
#define CENTRALSCHEDULER_H_

#include "AbstractTaskScheduler.h"
#include "helper/HwlocHelper.h"
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <taskscheduler/SharedScheduler.h>

class CentralScheduler;

// our worker thread objects
class WorkerThread {
private:
    CentralScheduler &scheduler;
public:
    WorkerThread(CentralScheduler &s) : scheduler(s) { }
    void operator()();
};


/**
 * a central scheduler holds a task queue and n worker threads
 */
class CentralScheduler : public AbstractTaskScheduler, public TaskReadyObserver {
  friend class WorkerThread;
  typedef std::unordered_set<std::shared_ptr<Task> > waiting_tasks_t;
  // set for tasks with open dependencies
  waiting_tasks_t _waitSet;
  // mutex to protect waitset
  std::mutex _setMutex;
  // queue of tasks that are ready to run
  std::queue<std::shared_ptr<Task> > _runQueue;
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
  CentralScheduler(int threads = getNumberOfCoresOnSystem());
  virtual ~CentralScheduler();

  void worker_loop();
  /*
   * schedule a task for execution
   */
  void schedule(std::shared_ptr<Task> task);
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  void shutdown();
  /*
   * resize the number of worker threads/queues
   */
  void resize(const size_t queues);
  /**
   * get number of worker
   */
  size_t getNumberOfWorker() const;

  void notifyReady(std::shared_ptr<Task> task);

};

#endif /* CENTRALSCHEDULER_H_ */
