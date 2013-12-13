/*
 * CoreBoundPriorityQueue.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "CoreBoundPriorityQueue.h"

#include <iostream>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <sched.h>

namespace hyrise {
namespace taskscheduler {

void CoreBoundPriorityQueue::executeTask() {

  //infinite thread loop
  while (1) {
    //block protected by _threadStatusMutex
    {
      std::lock_guard<lock_t> lk1(_threadStatusMutex);
      if (_status == TO_STOP)
        break;
    }
    // get task and execute
    std::shared_ptr<Task> task;
    _runQueue.try_pop(task);
    if (task) {
      // set queue to _blocked as we run task; this is a simple mechanism to avoid that further tasks are pushed to this queue if a long running task is executed; check WSSimpleTaskScheduler for task stealing queue
      _blocked = true;
      //LOG4CXX_DEBUG(logger, "Started executing task" << std::hex << &task << std::dec << " on core " << _core);
      // run task
      (*task)();
      //std::cout << "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;

      LOG4CXX_DEBUG(logger, "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core);
      // notify done observers that task is done
      task->notifyDoneObservers();
      _blocked = false;
    }  // no task in runQueue -> sleep and wait for new tasks
    
    // CoreBoundPriorityQueue based on tbb's queue keeps spinning for the time beeing;
    // we cannot trust _runQueue.size() and if we implement our own size(), we can use
    // a mutexed std::priority_queue 

    /*
    else {
      std::unique_lock<lock_t> ul(_queueMutex);
      //if queue still empty go to sleep and wait until new tasks have been arrived
      if (_runQueue.size() < 1) {
        // if thread is about to stop, break execution loop
        if(_status != RUN)
          break;
        //std::cout << "queue " << _core << " sleeping " << std::endl;
        _condition.wait(ul);        
      }
    }
    */
  }
}

CoreBoundPriorityQueue::CoreBoundPriorityQueue(int core): AbstractCoreBoundQueue(), _blocked(false) {
  _core = core;
  launchThread(_core);
}

void CoreBoundPriorityQueue::push(std::shared_ptr<Task> task) {
 // std::cout << "TASKQUEUE: task: "  << std::hex << (void * )task.get() << std::dec << " pushed to queue " << _core << std::endl;
  _runQueue.push(task);
  //_condition.notify_one();
}

std::vector<std::shared_ptr<Task> > CoreBoundPriorityQueue::stopQueue() {
  if (_status != STOPPED) {
    // the thread to be stopped is either executing a task, or waits for the condition variable
    // set status to "TO_STOP" so that the thread either quits after executing the task, or after having been notified by the condition variable
    {
      std::lock_guard<lock_t> lk(_queueMutex);
      _status = TO_STOP;
      //wake up thread in case thread is sleeping
      _condition.notify_one();
    }
    _thread->join();
    delete _thread;
    //just to make sure it points to nullptr
    _thread = nullptr;
    _status = STOPPED;

  }
  return emptyQueue();
}

std::vector<std::shared_ptr<Task> > CoreBoundPriorityQueue::emptyQueue() {
  // create empty queue
  std::vector<std::shared_ptr<Task> > tmp;
  std::lock_guard<lock_t> lk(_queueMutex);

  //move all elements to vector
  std::shared_ptr<Task> task;
  for(size_t i = 0, size = _runQueue.size(); i < size; i++){
    _runQueue.try_pop(task);
    tmp.push_back(task);
  }
  // return empty queue
  return tmp;
}

CoreBoundPriorityQueue::~CoreBoundPriorityQueue() {
  if (_thread != nullptr) stopQueue();
}

} } // namespace hyrise::taskscheduler

