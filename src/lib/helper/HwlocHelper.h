// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
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
