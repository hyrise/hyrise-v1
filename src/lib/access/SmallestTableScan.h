// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SMALLESTTABLESCAN_H_
#define SRC_LIB_ACCESS_SMALLESTTABLESCAN_H_

#include "PlanOperation.h"

namespace hyrise {
namespace access {

class SmallestTableScan : public _PlanOperation {
public:
  ~SmallestTableScan();

  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
};

}
}

#endif  // SRC_LIB_ACCESS_SMALLESTTABLESCAN_H_
