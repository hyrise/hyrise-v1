// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * HwlocHelper.h
 *
 *  Created on: Dec 4, 2012
 *      Author: jwust
 */

#include <hwloc.h>
#include "HwlocHelper.h"

int getNumberOfCoresOnSystem(){
  hwloc_topology_t topology = getHWTopology();
  static int NUM_PROCS = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
  return NUM_PROCS;
}

hwloc_topology_t getHWTopology(){
  static hwloc_topology_t topology;
  if (topology == nullptr) {
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
  }
  return topology;
}

