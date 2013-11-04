// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SIMPLETABLESCAN_H_
#define SRC_LIB_ACCESS_SIMPLETABLESCAN_H_

#include "access/system/ParallelizablePlanOperation.h"
#include "access/expressions/pred_SimpleExpression.h"

namespace hyrise {
namespace access {

class SimpleTableScan : public ParallelizablePlanOperation {
public:
  SimpleTableScan();
  virtual ~SimpleTableScan();

  void setupPlanOperation();
  void executePlanOperation();
  void executePositional();
  void executeMaterialized();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setPredicate(SimpleExpression *c);

private:
  SimpleExpression *_comparator;
  bool _ofDelta = false;
};

}
}

#endif  // SRC_LIB_ACCESS_SIMPLETABLESCAN_H_
