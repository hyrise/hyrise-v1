#ifndef SRC_LIB_ACCESS_SIMPLERAWTABLESCAN_H_
#define SRC_LIB_ACCESS_SIMPLERAWTABLESCAN_H_

#include "access/PlanOperation.h"
#include "access/pred_SimpleExpression.h"

class SimpleRawTableScan : public _PlanOperation {
 public:
  SimpleRawTableScan(SimpleExpression* comp, bool materializing=true);
  void setupPlanOperation();
  void executePlanOperation();
  const std::string vname();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
 private:
  // Comparison Field
  std::unique_ptr<SimpleExpression> _comparator;
  const bool _materializing;
};

#endif  // SRC_LIB_ACCESS_SIMPLERAWTABLESCAN_H_

