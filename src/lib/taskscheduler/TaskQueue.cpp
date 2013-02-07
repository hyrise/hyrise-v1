// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * TaskQueue.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#include "taskscheduler/TaskQueue.h"

#include <thread>
#include <queue>
#include <pthread.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>

#include <hwloc.h>

log4cxx::LoggerPtr TaskQueue::logger(log4cxx::Logger::getLogger("taskscheduler.TaskQueue"));

void TaskQueue::launchThread(int core) {
  static hwloc_topology_t topology;;
  if (topology == nullptr) {
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
  }

  //get the number of cores on system
  static int NUM_PROCS = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  if (core < NUM_PROCS) {
    _thread = new std::thread(&TaskQueue::executeTask, this);
    hwloc_cpuset_t cpuset;
    hwloc_obj_t obj;

    obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, core);
    // the bitmap to modify
    cpuset = hwloc_bitmap_dup(obj->cpuset);
    // remove hyperthreads
    hwloc_bitmap_singlify(cpuset);
    // bind
    if (hwloc_set_thread_cpubind(topology, _thread->native_handle(), cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND)) {
      char *str;
      int error = errno;
      hwloc_bitmap_asprintf(&str, obj->cpuset);
      printf("Couldn't bind to cpuset %s: %s\n", str, strerror(error));
      free(str);
      throw std::runtime_error(strerror(error));
    }

    hwloc_bitmap_free(cpuset);

  } else {
    // this case should never happen, as TaskQueue is only initialized from SimpleTaskScheduler, which captures this case
    throw std::logic_error("CPU to run thread on is larger than number of total cores; seems that TaskQueue was initialized outside of SimpleTaskScheduler, which should not happen");
  }
}

void TaskQueue::executeTask() {






  //infinite thread loop
  while (1) {

    //block protected by _threadStatusMutex, stop thread if about to stop
    {
      std::lock_guard<std::mutex> lk1(_threadStatusMutex);
      if (_status == TO_STOP)
        return;
    }

    std::unique_lock<std::mutex> ul(_queueMutex);
    // get task and execute
    if (_runQueue.size() > 0) {
      _blocked = true;
      std::shared_ptr<Task> task = _runQueue.front();
      _runQueue.pop();
      ul.unlock();
      if (task) {
        LOG4CXX_DEBUG(logger, "Started executing task" << std::hex << &task << std::dec << " on core " << _core);
        (*task)();
        LOG4CXX_DEBUG(logger, "Executed task " << std::hex << &task << std::dec << " on core " << _core);
        task->notifyDoneObservers();
      }
    }
    // TODO wait until new task/steal work
    else {
      {
        std::lock_guard<std::mutex> lk1(_threadStatusMutex);
        if (_status == RUN_UNTIL_DONE || _status == TO_STOP)
          return;
      }
      _blocked = false;
      _condition.wait(ul);
    }
  }
}

TaskQueue::TaskQueue(int core): _blocked(false), _status(RUN) {
  _core = core;
  launchThread(_core);
}

void TaskQueue::push(std::shared_ptr<Task> task) {
  std::lock_guard<std::mutex> lk(_queueMutex);
  _runQueue.push(task);
  _condition.notify_one();
}

std::queue<std::shared_ptr<Task> > *TaskQueue::stopQueue() {

  if (_status != STOPPED) {
    _threadStatusMutex.lock();
    // the thread to be stopped is either executing a task, or waits for the condition variable
    // set status to "TO_STOP" so that the thread either quits after executing the task, or after having been notified by the condition variable
    _status = TO_STOP;
    //wake up thread in case thread is sleeping
    _condition.notify_one();
    _threadStatusMutex.unlock();
    _thread->join();
    delete _thread;
    _thread = NULL;
    _status = STOPPED;
    return &_runQueue;
  } else
    // queue already stopped
    return NULL;
}

void TaskQueue::join() {
  _threadStatusMutex.lock();
  _status = RUN_UNTIL_DONE;
  _threadStatusMutex.unlock();
  _thread->join();
}

TaskQueue::~TaskQueue() {
  if (_thread != NULL) stopQueue();
}
