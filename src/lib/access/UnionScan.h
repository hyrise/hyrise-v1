// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_UNIONSCAN_H_
#define SRC_LIB_ACCESS_UNIONSCAN_H_

#include "access/PlanOperation.h"

namespace hyrise { namespace access {

class UnionScan : public _PlanOperation {
 public:
  void executePlanOperation();
};

}}

#endif  // SRC_LIB_ACCESS_UNIONSCAN_H_
