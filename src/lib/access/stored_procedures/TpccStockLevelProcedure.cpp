// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccStockLevelProcedure.h"

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

auto _ = net::Router::registerRoute<TpccStockLevelProcedure>(
             "/TPCC-StockLevel/");


TpccStockLevelProcedure::TpccStockLevelProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccStockLevelProcedure::setData(Json::Value& data) {
  _w_id = data["W_ID"].asInt();
  _d_id = data["D_ID"].asInt();
  _threshold = data["threshold"].asInt();
}

std::string TpccStockLevelProcedure::name() {
  return "TPCC-StockLevel";
}

const std::string TpccStockLevelProcedure::vname() {
  return "TPCC-StockLevel";
}

Json::Value TpccStockLevelProcedure::execute() {
  // Transaction
  _tx = startTransaction();

  auto t1 = getOId();
  _next_o_id = t1->getValue<int>("D_NEXT_O_ID", 0);

  auto t2 = getStockCount();

  commit(_tx);

  // Output
  int low_stock = 0;
  if (t2->size() != 0) 
    low_stock = t2->getValue<int>(0, 0);

  Json::Value result;
  result["W_ID"] = _w_id;
  result["D_ID"] = _d_id;
  result["threshold"] = _threshold;
  result["low_stock"] = low_stock;

  return result;
}

storage::c_atable_ptr_t TpccStockLevelProcedure::getOId() {
  auto district = loadTpccTable("DISTRICT", _tx);

  auto expr1 = new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id);
  auto expr2 = new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id);
  auto expr_and = new CompoundExpression(expr1, expr2, AND);
  auto validated = selectAndValidate(district, expr_and, _tx);

  auto result = project(validated, {"D_NEXT_O_ID"}, _tx);

  return result;
}

storage::c_atable_ptr_t TpccStockLevelProcedure::getStockCount() {
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
  auto expr_s2 = new LessThanExpression<hyrise_int_t>(stock, "S_QUANTITY", _threshold);
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

}} // namespace hyrise::access

