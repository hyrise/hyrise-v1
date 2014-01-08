/*
 * AbstractCoreBoundQueue.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "AbstractCoreBoundQueue.h"
#include <pthread.h>
#include <cstdlib>
#include <taskscheduler/AbstractCoreBoundQueue.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <helper/HwlocHelper.h>

namespace hyrise {
namespace taskscheduler {

log4cxx::LoggerPtr AbstractCoreBoundQueue::logger(log4cxx::Logger::getLogger("taskscheduler.AbstractCoreBoundQueue"));


AbstractCoreBoundQueue::AbstractCoreBoundQueue(): _status(RUN){
  // TODO Auto-generated constructor stub

}

AbstractCoreBoundQueue::~AbstractCoreBoundQueue() {
  // TODO Auto-generated destructor stub
}

void AbstractCoreBoundQueue::join() {
  _status = RUN_UNTIL_DONE;
  _condition.notify_one();
  _thread->join();
}

void AbstractCoreBoundQueue::launchThread(int core) {
  //get the number of cores on system
  int NUM_PROCS = getNumberOfCoresOnSystem();

  // NEver ever run antything on core 0, this is where the system runs
  // and we can only get worse from there thatswhy we use numprocs-1 as the suitable number
  
  const size_t freeCores = std::min(NUM_PROCS - 1, 2);
  core = (core % (NUM_PROCS - freeCores)) + freeCores;

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
      free(str);
    }
    // assuming single machine system                                                                                                         
    obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_MACHINE, 0);
    // set membind policy interleave for this thread                                                                                          
    if (hwloc_set_membind_nodeset(topology, obj->nodeset, HWLOC_MEMBIND_INTERLEAVE, HWLOC_MEMBIND_STRICT | HWLOC_MEMBIND_THREAD)) {
      char *str;
      int error = errno;
      hwloc_bitmap_asprintf(&str, obj->nodeset);
      fprintf(stderr, "Couldn't membind to nodeset  %s: %s\n", str, strerror(error));
      fprintf(stderr, "Continuing as normal, however, no guarantees\n");
      free(str);
    }

    hwloc_bitmap_free(cpuset);

  } else {
    // this case should never happen, as TaskQueue is only initialized from SimpleTaskScheduler, which captures this case
    throw std::logic_error("CPU to run thread on is larger than number of total cores; seems that TaskQueue was initialized outside of SimpleTaskScheduler, which should not happen");
  }
}

} } // namespace hyrise::taskscheduler

