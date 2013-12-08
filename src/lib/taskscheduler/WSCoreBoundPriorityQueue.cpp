/*
 * WSCoreBoundPriorityQueue.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "WSCoreBoundPriorityQueue.h"

namespace hyrise {
namespace taskscheduler {

WSCoreBoundPriorityQueue::WSCoreBoundPriorityQueue(int core, WSCoreBoundPriorityQueuesScheduler *scheduler): AbstractCoreBoundQueue(), _allQueues(nullptr) {
  _core = core;
  _scheduler = scheduler;
  launchThread(_core);
}

WSCoreBoundPriorityQueue::~WSCoreBoundPriorityQueue() {
  if (_thread != nullptr) stopQueue();
}

void WSCoreBoundPriorityQueue::executeTask() {
  //infinite thread loop
  while (1) {
    if (_status == TO_STOP)
      break;
    // get task and execute
    std::shared_ptr<Task> task;
    _runQueue.try_pop(task);

    // no task in runQueue -> try to steal task from other queue, otherwise sleep and wait for new tasks
    if(!task) {
      // try to steal work
      task = stealTasks();

      // WSCoreBoundPriorityQueue based on tbb's queue keeps spinning for the time beeing;
      // we cannot trust _runQueue.size() and if we implement our own size(), we can use
      // a mutexed std::priority_queue 
      
      /*
      if (!task){
        //if queue still empty go to sleep and wait until new tasks have been arrived
        std::unique_lock<lock_t> ul(_queueMutex);
        if (_runQueue.size() < 1) {
          {
            // if thread is about to stop, break execution loop
            if(_status != RUN)
              break;
            _condition.wait(ul);            
          }
        }
      }*/
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

std::shared_ptr<Task> WSCoreBoundPriorityQueue::stealTasks() {
  std::shared_ptr<Task> task = nullptr;
  if (_allQueues != nullptr) {
    int number_of_queues = _allQueues->size();
    if(number_of_queues > 1){
      // steal from the next queue (we only check number_of_queues -1, as we do not have to check the queue taht wants to steal)
      for (int i = 1; i < number_of_queues; i++) {
        // we steal relative from the current queue to distribute stealing over queues
        task = static_cast<WSCoreBoundPriorityQueue *>(_allQueues->at((i + _core) % number_of_queues))->stealTask();
        if (task != nullptr) {
          //push(task);
          //std::cout << "Queue " << _core << " stole Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " from queue " << i << std::endl;
          break;
        }
      }
    }
  }
  return task;
}

std::shared_ptr<Task> WSCoreBoundPriorityQueue::stealTask() {
  std::shared_ptr<Task> task = nullptr;
  // first check if status of thread is still ok;
  // dont steal tasks if thread is about to stop
  if (_status == RUN && _runQueue.size() >= 1) {
    // acquire queue mutex to check size and pop task
    _runQueue.try_pop(task);
  }
  return task;
}


void WSCoreBoundPriorityQueue::push(std::shared_ptr<Task> task) {
  // mutex is bad! but apparently we need it, otherwise, threads do not know whether they can sleep, cause runqueue.size may be incorrect if not synced
  std::lock_guard<lock_t> lk(_queueMutex);
  _runQueue.push(task);
  _condition.notify_one();
}

std::vector<std::shared_ptr<Task> > WSCoreBoundPriorityQueue::stopQueue() {
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

std::vector<std::shared_ptr<Task> > WSCoreBoundPriorityQueue::emptyQueue() {
  std::vector<std::shared_ptr<Task> > tmp;
  std::shared_ptr<Task> task;
  std::lock_guard<lock_t> lk(_queueMutex);
  while(!_runQueue.empty())
    _runQueue.try_pop(task);
    tmp.push_back(task);
  return tmp;
}

void WSCoreBoundPriorityQueue::refreshQueues(){
  _allQueues = _scheduler->getTaskQueues();
}

} } // namespace hyrise::taskscheduler

