// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SMALLESTTABLESCAN_H_
#define SRC_LIB_ACCESS_SMALLESTTABLESCAN_H_

#include "PlanOperation.h"

class SmallestTableScan : public _PlanOperation {
 public:

  SmallestTableScan() {
    
  }

  ~SmallestTableScan() {
    
  }

  void setupPlanOperation();
  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "SmallestTableScan";
  }
  const std::string vname() {
    return "SmallestTableScan";
  }
};

#endif  // SRC_LIB_ACCESS_SMALLESTTABLESCAN_H_

