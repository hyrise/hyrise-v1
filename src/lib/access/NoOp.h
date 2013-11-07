// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_NOOP_H_
#define SRC_LIB_ACCESS_NOOP_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class NoOp : public PlanOperation {
public:
  void executePlanOperation();
};

}}

#endif  // SRC_LIB_ACCESS_NOOP_H_
