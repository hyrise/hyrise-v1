/*
 * WSCoreBoundTaskQueue.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#include "taskscheduler/WSCoreBoundTaskQueue.h"

#include <thread>
#include <queue>
#include <pthread.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <cstdlib>

void WSCoreBoundTaskQueue::executeTask() {

  //infinite thread loop
  while (1) {
    std::shared_ptr<Task> task;

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
      // get first task
      task = _runQueue.front();
      _runQueue.pop_front();
      ul.unlock();
      if (task) {
        //LOG4CXX_DEBUG(logger, "Started executing task" << std::hex << &task << std::dec << " on core " << _core);
        // run task
        (*task)();
        //std::cout << "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;
        LOG4CXX_DEBUG(logger, "Executed task " << std::hex << &task << std::dec << " on core " << _core);
        // notify done observers that task is done
        task->notifyDoneObservers();
      }
      // no task in runQueue -> try to steal task from other queue, otherwise sleep and wait for new tasks
    } else {
      // try to steal work
      ul.unlock();
      if (stealTasks() != NULL)
        continue;
      ul.lock();
      //if queue still empty go to sleep and wait until new tasks have been arrived
      if (_runQueue.size() < 1) {
        {
          //check if queue was suspended
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
}

std::shared_ptr<Task> WSCoreBoundTaskQueue::stealTasks() {
  std::shared_ptr<Task> task = NULL;
  //check scheduler status
  WSSimpleTaskScheduler<WSCoreBoundTaskQueue>::scheduler_status_t status = _scheduler->getSchedulerStatus();
  if (status == WSSimpleTaskScheduler<WSCoreBoundTaskQueue>::RUN) {
    typedef typename WSSimpleTaskScheduler<WSCoreBoundTaskQueue>::task_queues_t task_queues_t;
    const task_queues_t *queues = _scheduler->getTaskQueues();
    if (queues != NULL) {
      int number_of_queues = queues->size();
      if(number_of_queues > 1){
        // steal from the next queue (we only check number_of_queues -1, as we do not have to check the queue taht wants to steal)
        for (int i = 1; i < number_of_queues; i++) {
	  // we steal relative from the current queue to distribute stealing over queues
	  task = static_cast<WSCoreBoundTaskQueue *>(queues->at((i + _core) % number_of_queues))->stealTask();
          if (task != NULL) {
            push(task);
            //std::cout << "Queue " << _core << " stole Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " from queue " << i << std::endl;
            break;
          }
        }
      }
    }
  }
  return task;
}

std::shared_ptr<Task> WSCoreBoundTaskQueue::stealTask() {
  std::shared_ptr<Task> task = NULL;
  // first check if status of thread is still ok; hold queueMutex, to avoid race conditions
  std::lock_guard<std::mutex> lk1(_queueMutex);
  // gather _threadStatusMutex, so thread cannot go to sleep
  std::lock_guard<std::mutex> lk2(_threadStatusMutex);
  // dont steal tasks if thread is about to stop
  if (_status == RUN) {
    // acquire queue mutex to check size and pop task
    if (_runQueue.size() >= 1) {
      task = _runQueue.back();
      _runQueue.pop_back();
    }
  }
  return task;
}

WSCoreBoundTaskQueue::WSCoreBoundTaskQueue(int core, WSSimpleTaskScheduler<WSCoreBoundTaskQueue> *scheduler): AbstractCoreBoundTaskQueue() {
  _core = core;
  _scheduler = scheduler;
  launchThread(_core);
}

void WSCoreBoundTaskQueue::push(std::shared_ptr<Task> task) {
  std::lock_guard<std::mutex> lk(_queueMutex);
  _runQueue.push_back(task);
  _condition.notify_one();
}

WSCoreBoundTaskQueue::run_queue_t WSCoreBoundTaskQueue::stopQueue() {
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
    _thread = NULL;
    _status = STOPPED;
  }
  return emptyQueue();
}

WSCoreBoundTaskQueue::run_queue_t WSCoreBoundTaskQueue::emptyQueue() {
  // create empty queue
  WSCoreBoundTaskQueue::run_queue_t tmp;
  std::lock_guard<std::mutex> lk(_queueMutex);
  //swap empty queue and _runQueue
  std::swap(tmp, _runQueue);
  // return empty queue
  return tmp;
}

WSCoreBoundTaskQueue::~WSCoreBoundTaskQueue() {
  if (_thread != NULL) stopQueue();
}



