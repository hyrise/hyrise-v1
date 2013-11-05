// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/TableScan.h"

#include "access/expressions/ExampleExpression.h"
#include "access/expressions/pred_SimpleExpression.h"
#include "access/expressions/ExpressionRegistration.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"
#include "helper/types.h"

namespace hyrise { namespace access {

namespace { auto _ = QueryParser::registerPlanOperation<TableScan>("TableScan"); }

TableScan::TableScan(std::unique_ptr<AbstractExpression> expr) : _expr(std::move(expr)) {}

void TableScan::setupPlanOperation() {
  const auto& table = getInputTable();
  _expr->walk({table});
}

void TableScan::executePlanOperation() {

  // When the input is 0, dont bother trying to generate results
  pos_list_t* positions = nullptr;
  if (getInputTable()->size() > 0) 
    positions = _expr->match(0, getInputTable()->size());
  else 
    positions = new pos_list_t();

  const auto& result = PointerCalculator::create(getInputTable(), positions);
  addResult(result);
}

std::shared_ptr<PlanOperation> TableScan::parse(const Json::Value& data) {
  return std::make_shared<TableScan>(Expressions::parse(data["expression"].asString(), data));
}

}}
