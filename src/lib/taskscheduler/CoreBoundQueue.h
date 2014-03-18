// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class Queue>
class CoreBoundQueue;
typedef CoreBoundQueue<PriorityQueueType> CoreBoundPriorityQueue;
typedef CoreBoundQueue<BasicQueueType> CoreBoundBasicQueue;

/*
* A Queue that asynchronously executes the tasks by threads
* that are bound to a given core.
*/
template <class QUEUE>
class CoreBoundQueue : virtual public ThreadLevelQueue<QUEUE> {
 protected:
  size_t _core;

 public:
  CoreBoundQueue(size_t core, size_t threads) : ThreadLevelQueue<QUEUE>(threads), _core(core) {}
  ~CoreBoundQueue() {}

  virtual void init() {
    for (size_t i = 0; i < ThreadLevelQueue<QUEUE>::_threadCount; i++) {
      launchThread(_core);
    }
    ThreadLevelQueue<QUEUE>::_status = AbstractTaskScheduler::RUN;
  }

 protected:
  void launchThread(int core) {
    std::thread* thread;
    // get the number of cores on system
    int NUM_PROCS = getNumberOfCoresOnSystem();

    // NEver ever run antything on core 0, this is where the system runs
    // and we can only get worse from there thatswhy we use numprocs-1 as the suitable number

    const size_t freeCores = std::min(NUM_PROCS - 1, 1);
    core = (core % (NUM_PROCS - freeCores)) + freeCores;

    if (core < NUM_PROCS) {
      // LOG4CXX_DEBUG(ThreadLevelQueue<QUEUE>::_logger, "Start worker thread and bind to core " << core);
      thread = new std::thread(&CoreBoundQueue<QUEUE>::executeTasks, this);
      hwloc_cpuset_t cpuset;
      hwloc_obj_t obj;
      hwloc_topology_t topology = getHWTopology();

      obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, core);
      // the bitmap to modify
      cpuset = hwloc_bitmap_dup(obj->cpuset);
      // remove hyperthreads
      hwloc_bitmap_singlify(cpuset);
      // bind
      if (hwloc_set_thread_cpubind(
              topology, thread->native_handle(), cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND)) {
        char* str;
        int error = errno;
        hwloc_bitmap_asprintf(&str, obj->cpuset);
        fprintf(stderr, "Couldn't bind to cpuset %s: %s\n", str, strerror(error));
        fprintf(stderr, "Continuing as normal, however, no guarantees\n");
        free(str);
      }
      // assuming single machine system
      obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_MACHINE, 0);
      // set membind policy interleave for this thread
      if (hwloc_set_membind_nodeset(
              topology, obj->nodeset, HWLOC_MEMBIND_INTERLEAVE, HWLOC_MEMBIND_STRICT | HWLOC_MEMBIND_THREAD)) {
        char* str;
        int error = errno;
        hwloc_bitmap_asprintf(&str, obj->nodeset);
        fprintf(stderr, "Couldn't membind to nodeset  %s: %s\n", str, strerror(error));
        fprintf(stderr, "Continuing as normal, however, no guarantees\n");
        free(str);
      }
      ThreadLevelQueue<QUEUE>::_threads.push_back(thread);
      hwloc_bitmap_free(cpuset);

    } else {
      // this case should never happen, as TaskQueue is only initialized from SimpleTaskScheduler, which captures this
      // case
      throw std::logic_error(
          "CPU to run thread on is larger than number of total cores; seems that TaskQueue was initialized outside of "
          "SimpleTaskScheduler, which should not happen");
    }
  }
};
}
}