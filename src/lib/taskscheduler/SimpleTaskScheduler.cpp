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
