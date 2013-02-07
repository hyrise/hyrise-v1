/*
 * HwlocHelper.h
 *
 *  Created on: Dec 4, 2012
 *      Author: jwust
 */

#ifndef HWLOCHELPER_H_
#define HWLOCHELPER_H_

#include <hwloc.h>

int getNumberOfCoresOnSystem();

hwloc_topology_t getHWTopology();


#endif /* HWLOCHELPER_H_ */
