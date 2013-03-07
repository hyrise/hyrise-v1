#include "SharedScheduler.h"
#include "WSSimpleTaskScheduler.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<WSSimpleTaskScheduler<WSCoreBoundTaskQueue> >("WSSimpleTaskScheduler");
}
