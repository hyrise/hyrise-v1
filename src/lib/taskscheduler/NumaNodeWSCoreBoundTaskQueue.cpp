/*
 * NumaNodeWSCoreBoundTaskQueue.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#include "taskscheduler/NumaNodeWSCoreBoundTaskQueue.h"
#include "access/PlanOperation.h"
#include <thread>
#include <queue>
#include <pthread.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <cstdlib>

NumaNodeWSCoreBoundTaskQueue::NumaNodeWSCoreBoundTaskQueue(int core, NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue> *scheduler){
  _node = getNodeForCore(core);
  _core = core;
  _scheduler = scheduler;
  launchThread(_core);
}

NumaNodeWSCoreBoundTaskQueue::~NumaNodeWSCoreBoundTaskQueue() {
  if (_thread != NULL) stopQueue();
}

void NumaNodeWSCoreBoundTaskQueue::executeTask() {
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

      // no task in runQueue -> try to steal task from other queue, otherwise sleep and wait for new tasks
    } else {
      // try to steal work
      ul.unlock();
      task = stealTasks();

      if (task == NULL){

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
    if (task) {
      //LOG4CXX_DEBUG(logger, "Started executing task" << std::hex << &task << std::dec << " on core " << _core);
      // run task
      //std::cout << "Running task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;
      task->setActualNode(_node);
      (*task)();
      //std::cout << "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;

      LOG4CXX_DEBUG(logger, "Executed task " << std::hex << &task << std::dec << " on core " << _core);
      // notify done observers that task is done
      task->notifyDoneObservers();
    }
  }
}

std::shared_ptr<Task> NumaNodeWSCoreBoundTaskQueue::stealTask() {
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

void NumaNodeWSCoreBoundTaskQueue::push(std::shared_ptr<Task> task) {
  std::lock_guard<std::mutex> lk(_queueMutex);
  _runQueue.push_back(task);
  _condition.notify_one();
}

NumaNodeWSCoreBoundTaskQueue::run_queue_t NumaNodeWSCoreBoundTaskQueue::stopQueue() {
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

NumaNodeWSCoreBoundTaskQueue::run_queue_t NumaNodeWSCoreBoundTaskQueue::emptyQueue() {
  // create empty queue
  NumaNodeWSCoreBoundTaskQueue::run_queue_t tmp;
  std::lock_guard<std::mutex> lk(_queueMutex);
  //swap empty queue and _runQueue
  std::swap(tmp, _runQueue);
  // return empty queue
  return tmp;
}


std::shared_ptr<Task> NumaNodeWSCoreBoundTaskQueue::stealTasks() {
  std::shared_ptr<Task> task = NULL;
  //check scheduler status
  NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue>::scheduler_status_t status = _scheduler->getSchedulerStatus();
  if (status == NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue>::RUN) {
    typedef typename NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue>::task_queues_t task_queues_t;
    typedef typename NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue>::task_queue_t task_queue_t;

    // try to steal from same Numa Node
    //std::cout << "Queue " << _core << " tries to steal tasks (node=" << _node <<")"<< std::endl;
    const task_queues_t queues = _scheduler->getTaskQueuesInSameNumaNode(_node);
    task_queue_t * queue;
    if (queues.size() > 0) {
      int number_of_queues = queues.size();
      for (int i = 0; i < number_of_queues; i++) {
        // we do not want to steal from ourself
        queue = queues.at(i);
        if(queue->getCore() != _core){
          task = static_cast<NumaNodeWSCoreBoundTaskQueue *>(queue)->stealTask();
          if (task != NULL){
            //std::cout << "Queue " << _core << " stole Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " from queue " << i << std::endl;
            break;
          }
        }
      }
    }
    // try to steal from other Numa nodes
    if(task == NULL){
      const task_queues_t queues = _scheduler->getTaskQueuesInOtherNumaNodes(_node);
      if (queues.size() > 0) {
        int number_of_queues = queues.size();
        for (int i = 0; i < number_of_queues; i++) {
          task = static_cast<NumaNodeWSCoreBoundTaskQueue *>(queues.at(i))->stealTask();
          if (task != NULL) {
            //push(task);
            //std::cout << "Queue " << _core << " stole Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " from queue " << queues.at(i)->getCore() << "(other numa node)"<< std::endl;
            break;
          }
        }
      }
    }
  }
  return task;
}

