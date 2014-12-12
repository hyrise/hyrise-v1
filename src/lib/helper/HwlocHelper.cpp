// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * HwlocHelper.h
 *
 *  Created on: Dec 4, 2012
 *      Author: jwust
 */

#include <hwloc.h>
#include "HwlocHelper.h"
#include <vector>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <memory>

int getNumberOfCoresOnSystem() {
  static int NUM_PROCS = []() {
    hwloc_topology_t topology = getHWTopology();
    return hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  }();
  assert(NUM_PROCS >= 1);
  return NUM_PROCS;
}


unsigned getNumberOfNodesOnSystem() {
  static int NUM_NODES = []() {
    hwloc_topology_t topology = getHWTopology();
    return std::max(hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE), 1);
  }();
  assert(NUM_NODES >= 1);
  return NUM_NODES;
}

hwloc_topology_t getHWTopology() {
  static std::unique_ptr<hwloc_topology, void (*)(hwloc_topology*)> topology = []() {
    hwloc_topology_t t;
    hwloc_topology_init(&t);
    hwloc_topology_load(t);
    return decltype(topology)(t, &hwloc_topology_destroy);
  }();
  return topology.get();
}

std::vector<unsigned> getCoresForNode(hwloc_topology_t topology, unsigned node) {
  std::vector<unsigned> children;

  // get number of cores by type
  unsigned number_of_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, node);
  if (obj == nullptr) {  // in case there is no node...
    children.resize(number_of_cores);
    std::iota(children.begin(), children.end(), 0);
    return children;
  }

  // get all cores and check whether core is in subtree of node, if yes, push to vector
  // iterate over cores and check whether in subtree
  for (unsigned i = 0; i < number_of_cores; i++) {
    hwloc_obj_t core = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, i);
    if (hwloc_obj_is_in_subtree(topology, core, obj)) {
      children.push_back(core->logical_index);
    }
  }
  return children;
}

std::vector<hwloc_cpuset_t> getCPUSetsForNode(hwloc_topology_t topology, unsigned node) {
  std::vector<hwloc_cpuset_t> children;

  // get number of cores by type
  unsigned number_of_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, node);
  if (obj == nullptr) {  // in case there is no node...
    children.resize(number_of_cores);
    for (unsigned i = 0; i < number_of_cores; i++) {
      hwloc_obj_t core = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, i);
      children.push_back(core->cpuset);
    }
    return children;
  }

  // get all cores and check whether core is in subtree of node, if yes, push to vector
  // iterate over cores and check whether in subtree
  for (unsigned i = 0; i < number_of_cores; i++) {
    hwloc_obj_t core = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, i);
    if (hwloc_obj_is_in_subtree(topology, core, obj)) {
      children.push_back(core->cpuset);
    }
  }
  return children;
}

unsigned getNumberOfNodes(hwloc_topology_t topology) { return hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE); }

signed getNodeForCore(unsigned core) {
  hwloc_topology_t topology = getHWTopology();
  unsigned nodes = getNumberOfNodes(topology);
  hwloc_obj_t core_obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, core);
  for (unsigned i = 0; i < nodes; i++) {
    hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, i);
    if (hwloc_obj_is_in_subtree(topology, core_obj, obj)) {
      return i;
    }
  }
  return -1;
}

// assumes equal number of cores per node
unsigned getNumberOfCoresPerNumaNode() {
  hwloc_topology_t topology = getHWTopology();
  int number_of_cores, number_of_nodes;
  number_of_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  number_of_nodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE);
  if (number_of_nodes > 0)
    return number_of_cores / number_of_nodes;
  if (number_of_nodes == 0)
    return number_of_cores;
  throw std::runtime_error("Multi-Level NUMA handling not implemented");
};

void bindCurrentThreadToCore(int core) {
  hwloc_topology_t topology = getHWTopology();
  hwloc_cpuset_t cpuset;
  hwloc_obj_t obj;

  // The actual core
  obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, core);
  cpuset = hwloc_bitmap_dup(obj->cpuset);
  hwloc_bitmap_singlify(cpuset);

  // bind
  if (hwloc_set_cpubind(topology, cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND | HWLOC_CPUBIND_THREAD)) {
    char* str;
    int error = errno;
    hwloc_bitmap_asprintf(&str, obj->cpuset);
    printf("Couldn't bind to cpuset %s: %s\n", str, strerror(error));
    free(str);
    throw std::runtime_error(strerror(error));
  }

  // free duplicated cpuset
  hwloc_bitmap_free(cpuset);

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
}

void bindCurrentThreadToNumaNode(int node) {
  hwloc_topology_t topology = getHWTopology();
  hwloc_cpuset_t cpuset;
  hwloc_obj_t obj;

  // The actual node
  obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, node);

  // obj is nullptr on non NUMA machines
  if (obj == nullptr) {
    fprintf(stderr, "Couldn't get hwloc object, bindCurrentThreadToNumaNode failed!\n");
    return;
  }

  cpuset = hwloc_bitmap_dup(obj->cpuset);
  // hwloc_bitmap_singlify(cpuset);

  // bind
  if (hwloc_set_cpubind(topology, cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND | HWLOC_CPUBIND_THREAD)) {
    char* str;
    int error = errno;
    hwloc_bitmap_asprintf(&str, obj->cpuset);
    printf("Couldn't bind to cpuset %s: %s\n", str, strerror(error));
    free(str);
    throw std::runtime_error(strerror(error));
  }

  // free duplicated cpuset
  hwloc_bitmap_free(cpuset);

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
}

signed getCurrentNode() {
  auto cur_core = getCurrentCore();
  if (cur_core != -1) {
    return getNodeForCore(cur_core);
  } else {
    return -1;
  }
}

signed getCurrentCore() {
  hwloc_topology_t topology = getHWTopology();
  hwloc_cpuset_t cpu_set = hwloc_bitmap_alloc();
  if (hwloc_get_last_cpu_location(topology, cpu_set, HWLOC_CPUBIND_THREAD) < 0) {
    return -1;
  }
  hwloc_obj_t current_core = hwloc_get_next_obj_covering_cpuset_by_type(topology, cpu_set, HWLOC_OBJ_CORE, NULL);
  hwloc_bitmap_free(cpu_set);
  return current_core->logical_index;
}
