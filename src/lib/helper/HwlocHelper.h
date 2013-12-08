// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * HwlocHelper.h
 *
 *  Created on: Dec 4, 2012
 *      Author: jwust
 */

#pragma once

#include <hwloc.h>
#include <vector>

int getNumberOfCoresOnSystem();
unsigned getNodeForCore(unsigned core);
hwloc_topology_t getHWTopology();
std::vector<unsigned> getCoresForNode(hwloc_topology_t topology, unsigned node);
unsigned getNumberOfNodes(hwloc_topology_t topology);
unsigned getNumberOfCoresPerNumaNode();

