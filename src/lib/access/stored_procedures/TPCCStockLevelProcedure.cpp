// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TPCCStockLevelProcedure.h"

#include <stdexcept>

#include <storage/AbstractTable.h>
#include <io/TransactionManager.h>
#include <access/storage/TableLoad.h>
#include <access/SimpleTableScan.h>
#include <access/ProjectionScan.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/JoinScan.h>
#include <access/GroupByScan.h>

#include <access/tx/ValidatePositions.h>
#include <access/tx/Commit.h>

namespace hyrise { namespace access {

TPCCStockLevelProcedure::TPCCStockLevelProcedure(int w_id, int d_id, int threshold) :
  _w_id(w_id),
  _d_id(d_id),
  _threshold(threshold) {
    // maybe check correctness of parameters
}

TPCCStockLevelProcedure::TPCCStockLevelProcedure() :
  _w_id(0),
  _d_id(0),
  _threshold(0) {
  throw std::runtime_error("cannot call this constructor for TPCCStockLevelProcedure");
}

TPCCStockLevelProcedure::TPCCStockLevelProcedure(const TPCCStockLevelProcedure& procedure) :
  _w_id(0),
  _d_id(0),
  _threshold(0) {
  throw std::runtime_error("cannot call this constructor for TPCCStockLevelProcedure");
}


result_map_t TPCCStockLevelProcedure::execute() {
  // Transaction
  _tx = startTransaction();

  auto t1 = getOId();
  _next_o_id = t1->getValue<int>("D_NEXT_O_ID", 0);
  t1->print();

  auto t2 = getStockCount();
  t2->print();

  commit(_tx);

  // Output

  int low_stock = 0;
  if (t2->size() != 0) 
    low_stock = t2->getValue<int>(0, 0);

  result_map_t result_map;
  result_map["W_ID"] = toString(_w_id);
  result_map["D_ID"] = toString(_d_id);
  result_map["threshold"] = toString(_threshold);
  result_map["low_stock"] = toString(low_stock);

  return result_map;
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::getOId() {
  auto district = loadAndValidate("DISTRICT", _tx);

  SimpleTableScan select;
  select.setTXContext(_tx);
  select.addInput(district);
  auto expr1 = new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id);
  auto expr2 = new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id);
  auto expr_and = new CompoundExpression(expr1, expr2, AND);
  select.setPredicate(expr_and);
  select.execute();

  auto result = project(select.getResultTable(), {"D_NEXT_O_ID"}, _tx);

  return result;
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::getStockCount() {
  auto order_line = loadAndValidate("ORDER_LINE", _tx);
  auto stock = loadAndValidate("STOCK", _tx);

  JoinScan join(JoinType::EQUI);
  join.addInput(order_line);
  join.addInput(stock);
  join.addJoinClause<int>(0, "OL_I_ID", 1, "S_I_ID");
  join.execute();
  auto joinResult = join.getResultTable();

  SimpleTableScan select;
  select.setTXContext(_tx);
  select.addInput(joinResult);
  {
    const auto min_o_id = _next_o_id - 21;
    const auto max_o_id = _next_o_id + 1; //D_NEXT_O_ID shoult be greater than every O_ID

    // comparators
    auto expr1 = new EqualsExpression<hyrise_int_t>(joinResult, "OL_W_ID", _w_id);
    auto expr2 = new EqualsExpression<hyrise_int_t>(joinResult, "OL_D_ID", _d_id);
    auto expr3 = new LessThanExpression<hyrise_int_t>(joinResult, "OL_O_ID", max_o_id);
    auto expr4 = new GreaterThanExpression<hyrise_int_t>(joinResult, "OL_O_ID", min_o_id);
    auto expr5 = new EqualsExpression<hyrise_int_t>(joinResult, "S_W_ID", _w_id);
    auto expr6 = new EqualsExpression<hyrise_int_t>(joinResult, "S_QUANTITY", _threshold);

    // ands
    auto expr_and1 = new CompoundExpression(expr1, expr2, AND);
    auto expr_and2 = new CompoundExpression(expr2, expr_and1, AND);
    auto expr_and3 = new CompoundExpression(expr3, expr_and2, AND);
    auto expr_and4 = new CompoundExpression(expr4, expr_and3, AND);
    auto expr_and5 = new CompoundExpression(expr5, expr_and4, AND);
    auto expr_and6 = new CompoundExpression(expr6, expr_and5, AND);
    select.setPredicate(expr_and6);
  }
  select.execute();

  GroupByScan groupby;
  groupby.addInput(select.getResultTable());
  auto count = new CountAggregateFun("OL_I_ID");
  groupby.addFunction(count);
  count->setDistinct(true);
  groupby.execute();

  return groupby.getResultTable();
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::loadAndValidate(std::string name, const tx::TXContext& tx) {
  TableLoad load;
  load.setTXContext(tx);
  load.setTableName(name);
  load.setHeaderFileName(tpccHeaderLocation.at(name));
  load.setFileName(tpccDataLocation.at(name));
  load.setDelimiter(tpccDelimiter);
  load.execute();
  
  ValidatePositions validate;
  validate.setTXContext(tx);
  validate.addInput(load.getResultTable());
  validate.execute();

  return validate.getResultTable();
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::project(storage::c_atable_ptr_t table, field_name_set_t fields, const tx::TXContext& tx) {
  ProjectionScan project;
  project.setTXContext(tx);
  project.addInput(table);
  for (auto& field : fields)
    project.addField(field);
  project.execute();
  return project.getResultTable();
}

tx::TXContext TPCCStockLevelProcedure::startTransaction() {
  return tx::TransactionManager::beginTransaction();
}

void TPCCStockLevelProcedure::commit(tx::TXContext tx) {
  Commit commit;
  commit.setTXContext(tx);
  commit.execute();
}

}} // namespace hyrise::access

