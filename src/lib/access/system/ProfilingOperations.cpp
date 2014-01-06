#include "access/system/ProfilingOperations.h"

#ifdef HYRISE_USE_GOOGLE_PROFILER
#include <gperftools/profiler.h>
#include <helper/Settings.h>
#endif

namespace hyrise {
namespace access {
namespace {
auto reg_pstart = QueryParser::registerTrivialPlanOperation<StartProfiling>("StartProfiling");
auto reg_pstop = QueryParser::registerTrivialPlanOperation<StopProfiling>("StopProfiling");
}

void StartProfiling::executePlanOperation() {
  output = input;
#ifdef HYRISE_USE_GOOGLE_PROFILER
  ProfilerStart(( Settings::getInstance()->getProfilePath() + "/profile_" + std::to_string(get_epoch_nanoseconds()) + ".gprof").c_str());
#endif
}

void StopProfiling::executePlanOperation() {
#ifdef HYRISE_USE_GOOGLE_PROFILER
  ProfilerStop();
#endif
  output = input;
}

}
}
