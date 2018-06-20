// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLESCAN_H_
#define SRC_LIB_ACCESS_TABLESCAN_H_

#include <memory>
#include "access/system/PlanOperation.h"
#include "access/PipelineObserver.h"
#include "access/PipelineEmitter.h"
#include "access/expressions/Expression.h"
#include "helper/types.h"


namespace hyrise {
namespace access {

/// Implements registration based expression scan
class PipeliningTableScan : public PlanOperation,
                            public PipelineObserver<PipeliningTableScan>,
                            public PipelineEmitter<PipeliningTableScan> {
 public:
  /// Construct TableScan for a specific expression, take
  /// ownership of passed in expression
  explicit PipeliningTableScan(std::unique_ptr<Expression> expr);
  /// Parse TableScan from
  const std::string vname() { return "PipeliningTableScan"; }
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

 protected:
  void setupPlanOperation();
  void executePlanOperation();
  virtual std::shared_ptr<AbstractPipelineObserver> clone() override;

 private:
  std::unique_ptr<Expression> _expr;
  storage::c_atable_ptr_t _table;
  void createAndEmitChunk(pos_list_t* positions);
};
}
}

#endif
