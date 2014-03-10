// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class Queue>
class NodeBoundQueue;
typedef NodeBoundQueue<PriorityQueueType> NodeBoundPriorityQueue;
typedef NodeBoundQueue<BasicQueueType> NodeBoundBasicQueue;

/*
* A Queue that asynchronously executes tasks by threads
* that are bound to a given node.
*/
template <class QUEUE>
class NodeBoundQueue : virtual public ThreadLevelQueue<QUEUE> {
 protected:
  size_t _node;

 public:
  NodeBoundQueue(size_t core, size_t threads) : ThreadLevelQueue<QUEUE>(threads), _node(core) {}
  ~NodeBoundQueue() {}

  virtual void init() {
    for (size_t i = 0; i < ThreadLevelQueue<QUEUE>::_threadCount; i++)
      launchThread(_node);
    ThreadLevelQueue<QUEUE>::_status = AbstractTaskScheduler::RUN;
  }

 protected:
  void launchThread(int node) {
    hwloc_obj_t obj;
    hwloc_topology_t topology = getHWTopology();
    // get ids for cores of the actual node
    std::vector<unsigned> cores = getCoresForNode(topology, _node);
    hwloc_cpuset_t cpuset;
    // allocate cpuset
    cpuset = hwloc_bitmap_alloc();
    // start thread
    std::thread* thread = new std::thread(&NodeBoundQueue<QUEUE>::executeTasks, this);

    // set all cores in cpuset
    for (size_t i = 0; i < cores.size(); i++) {
      hwloc_bitmap_set(cpuset, cores[i]);
    }

    // set affinity of thread to the node
    if (hwloc_set_thread_cpubind(
            topology, thread->native_handle(), cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND)) {
      char* str;
      int error = errno;
      hwloc_bitmap_asprintf(&str, cpuset);
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

    hwloc_bitmap_free(cpuset);
    ThreadLevelQueue<QUEUE>::_threads.push_back(thread);
  }
};
}
}