// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "ThreadLevelQueuesScheduler.h"
#include "SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

namespace {
bool registered1 =
    SharedScheduler::registerScheduler<ThreadLevelPriorityQueuesScheduler>("ThreadLevelPriorityQueuesScheduler");
bool registered2 = SharedScheduler::registerScheduler<ThreadLevelBasicQueuesScheduler>("ThreadLevelQueuesScheduler");
}
}
}