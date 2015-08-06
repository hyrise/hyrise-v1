// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "DynamicTaskQueue.h"
#include "SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

namespace {
bool registered1 = SharedScheduler::registerScheduler<DynamicTaskPriorityQueue>("DynamicPriorityScheduler");
bool registered2 = SharedScheduler::registerScheduler<DynamicTaskBasicQueue>("DynamicScheduler");
}
}
}
