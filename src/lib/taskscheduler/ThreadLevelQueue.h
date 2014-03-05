// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include <thread>
#include "AbstractTaskScheduler.h"
#include "PriorityQueueType.h"
#include "BasicQueueType.h"
#include <tbb/concurrent_queue.h>

namespace hyrise {
namespace taskscheduler {

template<class Queue> class ThreadLevelQueue;
typedef ThreadLevelQueue<PriorityQueueType> ThreadLevelPriorityQueue;
typedef ThreadLevelQueue<BasicQueueType> ThreadLevelBasicQueue;
typedef ThreadLevelQueue<tbb::concurrent_queue<std::shared_ptr<Task>>> TBBThreadLevelQueue;

/*
* A task queue with a number of threads executing tasks asynchronously;
* forms as a scheduler with a single central queue
*/
template<class QUEUE>
class ThreadLevelQueue : 
  public AbstractTaskScheduler,
  public TaskReadyObserver,
  public std::enable_shared_from_this<TaskReadyObserver> {
protected:
  QUEUE _runQueue;
  size_t _threadCount;
  std::vector<std::thread *> _threads;
  scheduler_status_t _status;
  static log4cxx::LoggerPtr _logger;
  bool _blocked;

public:
  ThreadLevelQueue(size_t threads):_threadCount(threads), _status(START_UP), _blocked(false) {}
  virtual ~ThreadLevelQueue(){
    if (_status != STOPPED) 
      shutdown();
  }

  virtual void init(){
    for(size_t i = 0; i < _threadCount; i++)
      launchThread();
    _status = RUN; 
  }

  /*
   * schedule a task for execution
   */
  virtual void schedule(std::shared_ptr<Task> task){
    task->lockForNotifications();
    if (task->isReady()) {
      task->unlockForNotifications();
      _runQueue.push(task);
    } else {
      task->addReadyObserver(shared_from_this());
    }
    task->unlockForNotifications();
  }

  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown(){
    if (_status != STOPPED) {
      _status = TO_STOP;
      for(size_t i = 0; i < _threadCount; i++){
        _threads[i]->join();
        delete _threads[i];
        //just to make sure it points to nullptr
        _threads[i] = nullptr;
      }
      _status = STOPPED;
    }
  }

  virtual size_t getNumberOfWorker() const{
    return _threadCount;
  }
  /*
  * returns true if queue is currently executing a task;
  * this is only meaningful for queues with a single thread
  */
  virtual bool blocked(){
    return _blocked;
  }

  virtual void notifyReady(std::shared_ptr<Task> task) {
    _runQueue.push(task);
  }

protected:  
  virtual void launchThread(){
    _threads.push_back(new std::thread(&ThreadLevelQueue<QUEUE>::executeTasks, this));
  }

  virtual void executeTasks(){
  //infinite thread loop
    while (true) {
      if (_status == TO_STOP) { break; } // break out when asked
      std::shared_ptr<Task> task = nullptr;
      if (_runQueue.try_pop(task)) {
        _blocked = true;
        (*task)();
        LOG4CXX_DEBUG(_logger,  "Executed task " << task->vname());
        task->notifyDoneObservers();
      } else {
        _blocked = false;
        std::this_thread::yield();
      }
    }
  }
};

template <class QUEUE>
log4cxx::LoggerPtr ThreadLevelQueue<QUEUE>::_logger = log4cxx::Logger::getLogger("taskscheduler.ThreadLevelQueue");


}}
