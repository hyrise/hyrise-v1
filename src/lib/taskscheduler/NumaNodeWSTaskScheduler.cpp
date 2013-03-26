#include "SharedScheduler.h"
#include "NumaNodeWSTaskScheduler.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue> >("NumaNodeWSTaskScheduler");
}
