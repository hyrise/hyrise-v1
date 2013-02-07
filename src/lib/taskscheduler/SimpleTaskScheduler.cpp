// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * SimpleTaskScheduler.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#include "SharedScheduler.h"
#include "SimpleTaskScheduler.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<SimpleTaskScheduler<CoreBoundTaskQueue> >("SimpleTaskScheduler");
}
