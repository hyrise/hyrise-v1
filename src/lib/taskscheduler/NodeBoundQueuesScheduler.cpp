// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "NodeBoundQueuesScheduler.h"
#include "SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

namespace {
bool registered1 =
    SharedScheduler::registerScheduler<NodeBoundPriorityQueuesScheduler>("NodeBoundPriorityQueuesScheduler");
bool registered2 = SharedScheduler::registerScheduler<NodeBoundBasicQueuesScheduler>("NodeBoundQueuesScheduler");
}
}
}
