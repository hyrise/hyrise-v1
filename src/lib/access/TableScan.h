#ifndef SRC_LIB_ACCESS_TABLESCAN_H_
#define SRC_LIB_ACCESS_TABLESCAN_H_

#include "access/PlanOperation.h"

class SimpleExpression;

namespace hyrise { namespace access {

class TableScan : public _PlanOperation {
 public:
  explicit TableScan(SimpleExpression* expr);
  ~TableScan();

  static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
  void setupPlanOperation();
  void executePlanOperation();
  const std::string vname() { return "me"; }
 private:
  SimpleExpression* _expr;
};

}}

#endif
