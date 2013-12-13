// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLESCAN_H_
#define SRC_LIB_ACCESS_TABLESCAN_H_

#include <memory>
#include "access/system/ParallelizablePlanOperation.h"
#include "helper/types.h"

namespace hyrise { namespace access {

class AbstractExpression;

/// Implements registration based expression scan
class TableScan : public ParallelizablePlanOperation {
 public:
  /// Construct TableScan for a specific expression, take
  /// ownership of passed in expression
  explicit TableScan(std::unique_ptr<AbstractExpression> expr);
  /// Parse TableScan from 
  const std::string vname() { return "TableScan"; }
  virtual std::vector<taskscheduler::task_ptr_t> applyDynamicParallelization(size_t dynamicCount);
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
 protected:
  void setupPlanOperation();
  void executePlanOperation();

  // for determineDynamicCount
  virtual size_t getTotalTableSize();
  virtual double min_mts_a() { return 0.00858530861116717; }
  virtual double min_mts_b() { return -0.0867733147921817; }
  virtual double a_a() { return 1.18977354704548; }
  virtual double a_b() { return -0.820926547220339; }
 private:
  std::unique_ptr<AbstractExpression> _expr;
};

}}

#endif
