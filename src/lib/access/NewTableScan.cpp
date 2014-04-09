// // Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
// #include "access/NewTableScan.h"
// #include "access/expressions/ExpressionLibrary.h"
// #include "storage/PointerCalculator.h"
// #include <iostream>

// namespace hyrise {
// namespace access {

// namespace {
// auto _ = QueryParser::registerPlanOperation<NewTableScan>("NewTableScan");
// }

// NewTableScan::NewTableScan(std::unique_ptr<Expression> expression) : _expression(std::move(expression)) {}

// std::shared_ptr<PlanOperation> NewTableScan::parse(const Json::Value& data) {
//   return std::make_shared<NewTableScan>(ExpressionLibrary::getInstance().dispatchAndParse(data));
// }

// void NewTableScan::executePlanOperation() {
//   std::shared_ptr<storage::PointerCalculator> result;

//   result = storage::PointerCalculator::create(getInputTable(), _expression->evaluate());

//   addResult(result);
// }

// void NewTableScan::setupPlanOperation() {
//   const auto& table = getInputTable();
  
//   _expression->setup(table);
// }
// }
// }