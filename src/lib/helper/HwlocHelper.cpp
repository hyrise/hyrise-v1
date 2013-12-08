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
#include <stdexcept>
#include <memory>

int getNumberOfCoresOnSystem(){
  hwloc_topology_t topology = getHWTopology();
  static int NUM_PROCS = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  return NUM_PROCS;
}

hwloc_topology_t getHWTopology(){
  static std::unique_ptr<hwloc_topology, void(*)(hwloc_topology*)> topology = []() {
    hwloc_topology_t t;
    hwloc_topology_init(&t);
    hwloc_topology_load(t);
    return decltype(topology)(t, &hwloc_topology_destroy);
  }();
  return topology.get();
}

std::vector<unsigned> getCoresForNode(hwloc_topology_t topology, unsigned node){
  std::vector<unsigned> children;
  unsigned number_of_cores;
  // get hwloc obj for node
  hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, node);
  // get all cores and check whether core is in subtree of node, if yes, push to vector
  // get number of cores by type
  number_of_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  // iterate over cores and check whether in subtree
  hwloc_obj_t core;
  for(unsigned i = 0; i < number_of_cores; i++){
    core = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, i);
    if(hwloc_obj_is_in_subtree(topology, core, obj)){
      children.push_back(core->logical_index);
    }
  }
  return children;
}

unsigned getNumberOfNodes(hwloc_topology_t topology){
  return hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE);
}

unsigned getNodeForCore(unsigned core){
  hwloc_topology_t topology = getHWTopology();
  unsigned nodes = getNumberOfNodes(topology);
  hwloc_obj_t obj;
  hwloc_obj_t core_obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, core);
  for(unsigned i = 0; i < nodes; i++){
    obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, i);
    if (hwloc_obj_is_in_subtree(topology, core_obj, obj)){
      return i;
    }
  }
  throw std::runtime_error("expected to find node for core");
}
//assumes equal number of cores per node
unsigned getNumberOfCoresPerNumaNode(){
  hwloc_topology_t topology = getHWTopology();
  unsigned number_of_cores, number_of_nodes;
  number_of_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  number_of_nodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE);
  return number_of_cores/number_of_nodes;
};
