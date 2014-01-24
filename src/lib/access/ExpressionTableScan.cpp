// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ExpressionTableScan.h"
#include "access/expressions/ExpressionLibrary.h"
#include "storage/PointerCalculator.h"
#include <iostream>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<ExpressionTableScan>("ExpressionTableScan");
}

ExpressionTableScan::ExpressionTableScan(std::unique_ptr<Expression> expression) : _expression(std::move(expression)) {}

std::shared_ptr<PlanOperation> ExpressionTableScan::parse(const Json::Value& data) {
  return std::make_shared<ExpressionTableScan>(ExpressionLibrary::getInstance().dispatchAndParse(data));
}

void ExpressionTableScan::executePlanOperation() {
  std::shared_ptr<storage::PointerCalculator> result;

  result = storage::PointerCalculator::create(getInputTable(), _expression->evaluate());

  addResult(result);
}

void ExpressionTableScan::setupPlanOperation() {
  const auto& table = getInputTable();
  
  _expression->setup(table);
}
}
}