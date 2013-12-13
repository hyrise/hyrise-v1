/*
 * CoreBoundQueue.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "CoreBoundQueue.h"

#include <iostream>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <sched.h>

namespace hyrise {
namespace taskscheduler {

void CoreBoundQueue::executeTask() {
  size_t retries = 0;
  //infinite thread loop
  while (1) {
    //block protected by _threadStatusMutex
    {
      std::lock_guard<lock_t> lk1(_threadStatusMutex);
      if (_status == TO_STOP)
        break;
    }
    // lock queue to get task
    std::unique_lock<lock_t> ul(_queueMutex);
    // get task and execute
    if (_runQueue.size() > 0) {
      std::shared_ptr<Task> task = _runQueue.front();
      // get first task
      _runQueue.pop();
      ul.unlock();
      retries = 0;
      if (task) {
        // set queue to _blocked as we run task; this is a simple mechanism to avoid that further tasks are pushed to this queue if a long running task is executed; check WSSimpleTaskScheduler for task stealing queue
        _blocked = true;
        //LOG4CXX_DEBUG(logger, "Started executing task" << std::hex << &task << std::dec << " on core " << _core);
        // run task
        //std::cout << "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;
        (*task)();
        LOG4CXX_DEBUG(logger, "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core);
        // notify done observers that task is done
        task->notifyDoneObservers();
        _blocked = false;
      }
    }
    // no task in runQueue -> sleep and wait for new tasks
    else {
      //if queue still empty go to sleep and wait until new tasks have been arrived
      if (_runQueue.size() < 1) {
        
        if (retries++ < 1000) {
          ul.unlock();
          if (retries > 300) 
            std::this_thread::yield();
        } else {
          // if thread is about to stop, break execution loop
          if(_status != RUN)
            break;
          _condition.wait(ul);      
        }
      }
    }
  }
}

CoreBoundQueue::CoreBoundQueue(int core): AbstractCoreBoundQueue(), _blocked(false) {
  _core = core;
  launchThread(_core);
}

void CoreBoundQueue::push(std::shared_ptr<Task> task) {
  //std::cout << "TASKQUEUE: task: "  << std::hex << (void * )task.get() << std::dec << " pushed to queue " << _core << std::endl;
  std::lock_guard<lock_t> lk(_queueMutex);
  _runQueue.push(task);
  _condition.notify_one();
}

std::vector<std::shared_ptr<Task> > CoreBoundQueue::stopQueue() {
  if (_status != STOPPED) {
    // the thread to be stopped is either executing a task, or waits for the condition variable
    // set status to "TO_STOP" so that the thread either quits after executing the task, or after having been notified by the condition variable
    {
      std::lock_guard<lock_t> lk(_queueMutex);
      {
        std::lock_guard<lock_t> lk(_threadStatusMutex);
        _status = TO_STOP;
      }
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

std::vector<std::shared_ptr<Task> > CoreBoundQueue::emptyQueue() {
  // create empty queue
  std::vector<std::shared_ptr<Task> > tmp;
  std::lock_guard<lock_t> lk(_queueMutex);

  //move all elements to vector
  for(size_t i = 0, size = _runQueue.size(); i < size; i++){
    tmp.push_back(_runQueue.front());
    _runQueue.pop();
  }
  // return empty queue
  return tmp;
}

CoreBoundQueue::~CoreBoundQueue() {
  if (_thread != nullptr) stopQueue();
}

} } // namespace hyrise::taskscheduler

