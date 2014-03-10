#ifndef SRC_LIB_ACCESS_SYSTEM_PROFILING_OPERATIONS_H
#define SRC_LIB_ACCESS_SYSTEM_PROFILING_OPERATIONS_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class StartProfiling : public PlanOperation {
  void executePlanOperation();
};

class StopProfiling : public PlanOperation {
  void executePlanOperation();
};
}
}

#endif
