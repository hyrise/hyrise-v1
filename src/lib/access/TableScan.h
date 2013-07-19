// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLESCAN_H_
#define SRC_LIB_ACCESS_TABLESCAN_H_

#include <memory>
#include "access/system/PlanOperation.h"

namespace hyrise { namespace access {

class AbstractExpression;

/// Implements registration based expression scan
class TableScan : public PlanOperation {
 public:
  /// Construct TableScan for a specific expression, take
  /// ownership of passed in expression
  explicit TableScan(std::unique_ptr<AbstractExpression> expr);
  /// Parse TableScan from 
  static std::shared_ptr<PlanOperation> parse(Json::Value& data);
  const std::string vname() { return "me"; }
 protected:
  void setupPlanOperation();
  void executePlanOperation();
 private:
  std::unique_ptr<AbstractExpression> _expr;
};

}}

#endif
