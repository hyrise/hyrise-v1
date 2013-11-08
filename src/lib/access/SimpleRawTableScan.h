// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SIMPLERAWTABLESCAN_H_
#define SRC_LIB_ACCESS_SIMPLERAWTABLESCAN_H_

#include "access/system/PlanOperation.h"
#include "access/expressions/pred_SimpleExpression.h"

namespace hyrise {
namespace access {

class SimpleRawTableScan : public PlanOperation {
public:
  SimpleRawTableScan(SimpleExpression *comp,
                     const bool materializing = true);
  virtual ~SimpleRawTableScan();

  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();

private:
  // Comparison Field
  std::unique_ptr<SimpleExpression> _comparator;
  const bool _materializing;
};

}
}

#endif  // SRC_LIB_ACCESS_SIMPLERAWTABLESCAN_H_
