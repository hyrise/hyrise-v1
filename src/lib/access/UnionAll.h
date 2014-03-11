#ifndef SRC_LIB_ACCESS_UNIONALL_H
#define SRC_LIB_ACCESS_UNIONALL_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class UnionAll : public PlanOperation {
  void executePlanOperation();
};
}
}

#endif
