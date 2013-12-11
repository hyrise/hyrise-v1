// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/TableScan.h"

#include "access/expressions/ExampleExpression.h"
#include "access/expressions/pred_SimpleExpression.h"
#include "access/expressions/ExpressionRegistration.h"
#include "storage/PointerCalculator.h"
#include "storage/TableRangeView.h"
#include "helper/types.h"

namespace hyrise { namespace access {

namespace { auto _ = QueryParser::registerPlanOperation<TableScan>("TableScan"); }

TableScan::TableScan(std::unique_ptr<AbstractExpression> expr) : _expr(std::move(expr)) {}

void TableScan::setupPlanOperation() {
  const auto& table = getInputTable();
  auto tablerange = std::dynamic_pointer_cast<const storage::TableRangeView>(table);
  if(tablerange)
    _expr->walk({tablerange->getActualTable()});
  else
    _expr->walk({table});
}

void TableScan::executePlanOperation() {
  size_t start, stop;
  const auto& tablerange = std::dynamic_pointer_cast<const storage::TableRangeView>(getInputTable());
  if(tablerange){
    start = tablerange->getStart();
    stop = start + tablerange->size();
  }
  else{
    start = 0;
    stop = getInputTable()->size();
  }


  // When the input is 0, dont bother trying to generate results
  pos_list_t* positions = nullptr;
  if(stop - start > 0)
    positions = _expr->match(start, stop);
  else
    positions = new pos_list_t();

  std::shared_ptr<storage::PointerCalculator> result;

  if(tablerange)
    result = storage::PointerCalculator::create(tablerange->getActualTable(), positions);
  else
    result = storage::PointerCalculator::create(getInputTable(), positions);

  addResult(result);
}

std::shared_ptr<PlanOperation> TableScan::parse(const Json::Value& data) {
  return std::make_shared<TableScan>(Expressions::parse(data["expression"].asString(), data));
}

}}
