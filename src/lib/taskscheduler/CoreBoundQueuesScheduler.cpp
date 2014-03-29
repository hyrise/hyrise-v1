// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "CoreBoundQueuesScheduler.h"
#include "SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

namespace {
bool registered1 =
    SharedScheduler::registerScheduler<CoreBoundPriorityQueuesScheduler>("CoreBoundPriorityQueuesScheduler");
bool registered2 = SharedScheduler::registerScheduler<CoreBoundBasicQueuesScheduler>("CoreBoundQueuesScheduler");
}
}
}
