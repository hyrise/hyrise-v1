// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SIMPLEPIPELININGTABLESCAN_H_
#define SRC_LIB_ACCESS_SIMPLEPIPELININGTABLESCAN_H_

#include "access/system/PlanOperation.h"
#include "access/expressions/pred_SimpleExpression.h"
#include "access/PipelineObserver.h"
#include "access/PipelineEmitter.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

class SimplePipeliningTableScan : public PlanOperation,
                                  public PipelineObserver<SimplePipeliningTableScan>,
                                  public PipelineEmitter<SimplePipeliningTableScan> {

 public:
  SimplePipeliningTableScan();
  virtual ~SimplePipeliningTableScan();

  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  virtual std::shared_ptr<PlanOperation> copy();
  void setPredicate(SimpleExpression* c);

 private:
  SimpleExpression* _comparator;
  Json::Value _predicates;
  bool _ofDelta = false;
  storage::pos_list_t* _pos_list = new pos_list_t();
  storage::c_atable_ptr_t _tbl = nullptr;
  void emitChunk();
};
}
}

#endif  // SRC_LIB_ACCESS_SIMPLEPIPELININGTABLESCAN_H_
