/*
 * WSCoreBoundQueue.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "WSCoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

WSCoreBoundQueue::WSCoreBoundQueue(int core, WSCoreBoundQueuesScheduler *scheduler): AbstractCoreBoundQueue() {
  _core = core;
  _scheduler = scheduler;
  launchThread(_core);
}

WSCoreBoundQueue::~WSCoreBoundQueue() {
  if (_thread != nullptr) stopQueue();
}

void WSCoreBoundQueue::executeTask() {
  //infinite thread loop
  while (1) {
    //block protected by _threadStatusMutex
    if (_status == TO_STOP)
      break;

    // lock queue to get task
    std::unique_lock<lock_t> ul(_queueMutex);

    std::shared_ptr<Task> task;
    // get task and execute
    if (_runQueue.size() > 0) {
      // get first task
      task = _runQueue.front();
      _runQueue.pop_front();
      ul.unlock();

      // no task in runQueue -> try to steal task from other queue, otherwise sleep and wait for new tasks
    } else {
      ul.unlock();
      // try to steal work
      task = stealTasks();
      if (!task){
        ul.lock();
        //if queue still empty go to sleep and wait until new tasks have been arrived
        if (_runQueue.size() < 1) {
          {
            // if thread is about to stop, break execution loop
            if(_status != RUN)
              break;
            _condition.wait(ul);           
          }
        }
      }
    }
    if (task) {
      //LOG4CXX_DEBUG(logger, "Started executing task" << std::hex << &task << std::dec << " on core " << _core);
      // run task
      //std::cout << "Running task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;
      (*task)();
      //std::cout << "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec << " on core " << _core<< std::endl;

      LOG4CXX_DEBUG(logger, "Executed task " << std::hex << &task << std::dec << " on core " << _core);
      // notify done observers that task is done
      task->notifyDoneObservers();
    }
  }
}

std::shared_ptr<Task> WSCoreBoundQueue::stealTasks() {
  std::shared_ptr<Task> task = nullptr;
  //check scheduler status
  WSCoreBoundQueuesScheduler::scheduler_status_t status = _scheduler->getSchedulerStatus();
  if (status == WSCoreBoundQueuesScheduler::RUN) {
    auto *queues = _scheduler->getTaskQueues();
    if (queues != nullptr) {
      int number_of_queues = queues->size();
      if(number_of_queues > 1){
        // steal from the next queue (we only check number_of_queues -1, as we do not have to check the queue taht wants to steal)
        for (int i = 1; i < number_of_queues; i++) {
          // we steal relative from the current queue to distribute stealing over queues
          task = static_cast<WSCoreBoundQueue *>(queues->at((i + _core) % number_of_queues))->stealTask();
          if (task != nullptr) {
            //push(task);
            //std::cout << "Queue " << _core << " stole Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " from queue " << i << std::endl;
            break;
          }
        }
      }
    }
  }
  return task;
}

std::shared_ptr<Task> WSCoreBoundQueue::stealTask() {
  std::shared_ptr<Task> task = nullptr;
  // hold queueMutex, to avoid race conditions
  std::lock_guard<lock_t> lk1(_queueMutex);
  // dont steal tasks if thread is about to stop
  if (_status == RUN && _runQueue.size() >= 1) {
      task = _runQueue.back();
      _runQueue.pop_back();
  }
  return task;
}

void WSCoreBoundQueue::push(std::shared_ptr<Task> task) {
  std::lock_guard<lock_t> lk(_queueMutex);
  _runQueue.push_back(task);
  _condition.notify_one();
}

std::vector<std::shared_ptr<Task> > WSCoreBoundQueue::stopQueue() {
  if (_status != STOPPED) {
    // the thread to be stopped is either executing a task, or waits for the condition variable
    // set status to "TO_STOP" so that the thread either quits after executing the task, or after having been notified by the condition variable
    // we need the mutex here, otherwise, we might call notify prior to the thread going to sleep
    {
      std::lock_guard<lock_t> lk(_queueMutex);
      _status = TO_STOP;
      //wake up thread in case thread is sleeping
      _condition.notify_one();
    }
    _thread->join();
    delete _thread;
    _thread = nullptr;
    _status = STOPPED;
  }
  return emptyQueue();
}

std::vector<std::shared_ptr<Task> > WSCoreBoundQueue::emptyQueue() {
  std::vector<std::shared_ptr<Task> > tmp;
  std::lock_guard<lock_t> lk(_queueMutex);
  for(auto it = _runQueue.begin(); it != _runQueue.end(); it++)
    tmp.push_back(*it);
  return tmp;
}

} } // namespace hyrise::taskscheduler

