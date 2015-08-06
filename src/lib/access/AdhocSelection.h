#pragma once

#include "access/system/ParallelizablePlanOperation.h"

namespace hyrise {
namespace access {

// Ad-hoc expression selection scan, allows to define
// arbitrary expression with a boolean result to scan
// a table row by row. The expressions must use column
// names of the incoming table. Supports all basic types
// of hyrise
// Warning: Operator is slow, but mostly complete in its
// support of all kinds of adhoc constructs.
class AdhocSelection : public ParallelizablePlanOperation {
 public:
  AdhocSelection(std::string expression) : _expression(expression) {}
  void executePlanOperation() override;
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

 private:
  // Expression string that evaluates to true or false for one row
  //
  // Examples: "A == B", "A > B", "A + B == C", "(A == 2) and (C == 1)"
  // For more examples, check the documentation of muparserX
  std::string _expression;
};
}
}
