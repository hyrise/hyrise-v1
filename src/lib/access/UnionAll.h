#ifndef SRC_LIB_ACCESS_UNIONALL_H
#define SRC_LIB_ACCESS_UNIONALL_H

#include "access/PlanOperation.h"

namespace hyrise { namespace access {

class UnionAll : public _PlanOperation {
  void executePlanOperation();
};

}}

#endif


