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
/*
 * Get the node for the core specificed by the supplied logical index.
 * Returns the logical index if a node was found.
 * Returns -1 if no node was found or if system has no nodes.
 */
signed getNodeForCore(unsigned core);
hwloc_topology_t getHWTopology();
std::vector<unsigned> getCoresForNode(hwloc_topology_t topology, unsigned node);
unsigned getNumberOfNodes(hwloc_topology_t topology);
unsigned getNumberOfCoresPerNumaNode();
void bindCurrentThreadToCore(int core);
void bindCurrentThreadToNumaNode(int node);
unsigned getNumberOfNodesOnSystem();
/*
 * Returns the core the calling thread last ran on.
 * The returned index can be used as a logical hwloc index for HWLOC_OBJ_CORE
 * objects.
 * It returns -1 if the lookup failed.
 */
signed getCurrentCore();
/*
 * Returns the last (NUMA) node the calling thread ran on.
 * Returns -1 if the lookup failed.
 */
signed getCurrentNode();
