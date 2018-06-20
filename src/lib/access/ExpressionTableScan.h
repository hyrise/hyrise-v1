// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSIONTABLESCAN_H_
#define SRC_LIB_ACCESS_EXPRESSIONTABLESCAN_H_

#include "access/system/ParallelizablePlanOperation.h"
#include "helper/types.h"
#include "access/expressions/Expression.h"

namespace hyrise {
namespace access {

class ExpressionTableScan : public ParallelizablePlanOperation {
 public:
  ExpressionTableScan(std::unique_ptr<Expression> Expression);

  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  void setupPlanOperation();
  void executePlanOperation();

 private:
  std::unique_ptr<Expression> _expression;
};
}
}

#endif
