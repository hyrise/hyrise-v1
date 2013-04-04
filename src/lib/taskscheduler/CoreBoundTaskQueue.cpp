// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * CoreBoundTaskQueue.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#include "taskscheduler/CoreBoundTaskQueue.h"

#include <iostream>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <sched.h>

void CoreBoundTaskQueue::executeTask() {

  //infinite thread loop
  while (1) {
    //block protected by _threadStatusMutex
    {
      std::lock_guard<std::mutex> lk1(_threadStatusMutex);
      if (_status == TO_STOP)
        break;
    }
    // lock queue to get task
    std::unique_lock<std::mutex> ul(_queueMutex);
    // get task and execute
    if (_runQueue.size() > 0) {
      std::shared_ptr<Task> task = _runQueue.front();
      // get first task
      _runQueue.pop();
      ul.unlock();
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
        // if thread is about to stop, break execution loop
        {
          std::lock_guard<std::mutex> lk1(_threadStatusMutex);
          if (_status != RUN)
            continue;
        }
        //std::cout << "queue " << _core << " sleeping " << std::endl;
        _condition.wait(ul);
      }
    }
  }
}

CoreBoundTaskQueue::CoreBoundTaskQueue(int core): AbstractCoreBoundTaskQueue(), _blocked(false) {
  _core = core;
  launchThread(_core);
}

void CoreBoundTaskQueue::push(std::shared_ptr<Task> task) {
  //std::cout << "TASKQUEUE: task: "  << std::hex << (void * )task.get() << std::dec << " pushed to queue " << _core << std::endl;
  std::lock_guard<std::mutex> lk(_queueMutex);
  _runQueue.push(task);
  _condition.notify_one();
}

std::queue<std::shared_ptr<Task> > CoreBoundTaskQueue::stopQueue() {
  if (_status != STOPPED) {
    // the thread to be stopped is either executing a task, or waits for the condition variable
    // set status to "TO_STOP" so that the thread either quits after executing the task, or after having been notified by the condition variable
    {
      std::lock_guard<std::mutex> lk(_queueMutex);
      {
        std::lock_guard<std::mutex> lk(_threadStatusMutex);
        _status = TO_STOP;
      }
      //wake up thread in case thread is sleeping
      _condition.notify_one();
    }
    _thread->join();
    delete _thread;
    //just to make sure it points to NULL
    _thread = NULL;

    _status = STOPPED;

  }
  return emptyQueue();
}

std::queue<std::shared_ptr<Task> > CoreBoundTaskQueue::emptyQueue() {
  // create empty queue
  std::queue<std::shared_ptr<Task> > tmp;
  std::lock_guard<std::mutex> lk(_queueMutex);
  //swap empty queue and _runQueue
  std::swap(tmp, _runQueue);
  // return empty queue
  return tmp;
}

CoreBoundTaskQueue::~CoreBoundTaskQueue() {
  if (_thread != NULL) stopQueue();
}
