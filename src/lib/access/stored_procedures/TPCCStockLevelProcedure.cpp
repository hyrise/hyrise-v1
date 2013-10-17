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
  auto district = loadTpccTable("DISTRICT", _tx);

  auto expr1 = new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id);
  auto expr2 = new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id);
  auto expr_and = new CompoundExpression(expr1, expr2, AND);
  auto validated = selectAndValidate(district, expr_and, _tx);

  auto result = project(validated, {"D_NEXT_O_ID"}, _tx);

  return result;
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::getStockCount() {
  // order_line: load, select and validate
  auto order_line = loadTpccTable("ORDER_LINE", _tx);
  const auto min_o_id = _next_o_id - 21;
  const auto max_o_id = _next_o_id + 1; //D_NEXT_O_ID shoult be greater than every O_ID

  auto expr_ol1 = new EqualsExpression<hyrise_int_t>(order_line, "OL_W_ID", _w_id);
  auto expr_ol2 = new EqualsExpression<hyrise_int_t>(order_line, "OL_D_ID", _d_id);
  auto expr_ol3 = new LessThanExpression<hyrise_int_t>(order_line, "OL_O_ID", max_o_id);
  auto expr_ol4 = new GreaterThanExpression<hyrise_int_t>(order_line, "OL_O_ID", min_o_id);

  auto expr_ol_and1 = new CompoundExpression(expr_ol1, expr_ol2, AND);
  auto expr_ol_and2 = new CompoundExpression(expr_ol3, expr_ol_and1, AND);
  auto expr_ol_and3 = new CompoundExpression(expr_ol4, expr_ol_and2, AND);

  auto validated_ol = selectAndValidate(order_line, expr_ol_and3, _tx);
 

  // stock:      load, select and validate
  auto stock = loadTpccTable("STOCK", _tx);
  auto expr_s1 = new EqualsExpression<hyrise_int_t>(stock, "S_W_ID", _w_id);
  auto expr_s2 = new EqualsExpression<hyrise_int_t>(stock, "S_QUANTITY", _threshold);
  auto expr_s_and = new CompoundExpression(expr_s1, expr_s2, AND);
  auto validated_s = selectAndValidate(stock, expr_s_and, _tx);

  JoinScan join(JoinType::EQUI);
  join.addInput(validated_ol);
  join.addInput(validated_s);
  join.addJoinClause<int>(0, "OL_I_ID", 1, "S_I_ID");
  join.execute();

  GroupByScan groupby;
  groupby.addInput(join.getResultTable());
  auto count = new CountAggregateFun("OL_I_ID");
  groupby.addFunction(count);
  count->setDistinct(true);
  groupby.execute();

  return groupby.getResultTable();
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::loadTpccTable(std::string name, const tx::TXContext& tx) {
  TableLoad load;
  load.setTXContext(tx);
  load.setTableName(name);
  load.setHeaderFileName(tpccHeaderLocation.at(name));
  load.setFileName(tpccDataLocation.at(name));
  load.setDelimiter(tpccDelimiter);
  load.execute();

  return load.getResultTable();
}
 
storage::c_atable_ptr_t TPCCStockLevelProcedure::selectAndValidate(storage::c_atable_ptr_t table, SimpleExpression *expr, const tx::TXContext& tx) {
  SimpleTableScan select;
  select.addInput(table);
  select.setTXContext(_tx);
  select.setPredicate(expr);
  select.execute();
  
  ValidatePositions validate;
  validate.setTXContext(tx);
  validate.addInput(select.getResultTable());
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

