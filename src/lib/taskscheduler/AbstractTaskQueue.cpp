/*
 * AbstractTaskQueue.cpp
 *
 *  Created on: Jun 13, 2012
 *      Author: jwust
 */

#include <pthread.h>
#include <cstdlib>
#include <taskscheduler/AbstractTaskQueue.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <helper/HwlocHelper.h>

log4cxx::LoggerPtr AbstractCoreBoundTaskQueue::logger(log4cxx::Logger::getLogger("taskscheduler.AbstractCoreBoundTaskQueue"));

void AbstractCoreBoundTaskQueue::join() {
  _threadStatusMutex.lock();
  _status = RUN_UNTIL_DONE;
  _threadStatusMutex.unlock();
  _thread->join();
}

void AbstractCoreBoundTaskQueue::launchThread(int core) {
  //get the number of cores on system
  int NUM_PROCS = getNumberOfCoresOnSystem();

  if (core < NUM_PROCS) {
    _thread = new std::thread(&AbstractTaskQueue::executeTask, this);
    hwloc_cpuset_t cpuset;
    hwloc_obj_t obj;
    hwloc_topology_t topology = getHWTopology();

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
      fprintf(stderr, "Couldn't bind to cpuset %s: %s\n", str, strerror(error));
      fprintf(stderr, "Continuing as normal, however, no guarantees\n");
      //throw std::runtime_error(strerror(error));
    }

    hwloc_bitmap_free(cpuset);

  } else {
    // this case should never happen, as TaskQueue is only initialized from SimpleTaskScheduler, which captures this case
    throw std::logic_error("CPU to run thread on is larger than number of total cores; seems that TaskQueue was initialized outside of SimpleTaskScheduler, which should not happen");
  }
}
