// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccStockLevelProcedure.h"

#include <storage/AbstractTable.h>
#include <access.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccStockLevelProcedure>("/TPCC-StockLevel/");
} // namespace


TpccStockLevelProcedure::TpccStockLevelProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccStockLevelProcedure::setData(Json::Value& data) {
  _w_id = assureMemberExists(data, "W_ID").asInt();
  _d_id = assureMemberExists(data, "D_ID").asInt();
  _threshold = assureMemberExists(data, "threshold").asInt();
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
  auto district = getTpccTable("DISTRICT", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions), _tx);

  auto result = project(validated, {"D_NEXT_O_ID"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccStockLevelProcedure::getStockCount() {
  // order_line: load, select and validate
  auto order_line = getTpccTable("ORDER_LINE", _tx);
  const auto min_o_id = _next_o_id - 21;
  const auto max_o_id = _next_o_id + 1; //D_NEXT_O_ID shoult be greater than every O_ID

  expr_list_t expressions_ol;
  expressions_ol.push_back(new EqualsExpression<hyrise_int_t>(order_line, "OL_W_ID", _w_id));
  expressions_ol.push_back(new EqualsExpression<hyrise_int_t>(order_line, "OL_D_ID", _d_id));
  expressions_ol.push_back(new LessThanExpression<hyrise_int_t>(order_line, "OL_O_ID", max_o_id));
  expressions_ol.push_back(new GreaterThanExpression<hyrise_int_t>(order_line, "OL_O_ID", min_o_id));
  auto validated_ol = selectAndValidate(order_line, connectAnd(expressions_ol), _tx);

  auto stock = getTpccTable("STOCK", _tx);
  expr_list_t expressions_s;
  expressions_s.push_back(new EqualsExpression<hyrise_int_t>(stock, "S_W_ID", _w_id));
  expressions_s.push_back(new LessThanExpression<hyrise_int_t>(stock, "S_QUANTITY", _threshold));
  auto validated_s = selectAndValidate(stock, connectAnd(expressions_s), _tx);

  JoinScan join(JoinType::EQUI);
  join.addInput(validated_ol);
  join.addInput(validated_s);
  join.addJoinClause<int>(0, "OL_I_ID", 1, "S_I_ID");
  join.execute();

  GroupByScan groupby;
  groupby.addInput(join.getResultTable());
  auto count = new CountAggregateFun("OL_I_ID");
  count->setDistinct(true);
  groupby.addFunction(count);
  groupby.execute();

  return groupby.getResultTable();
}

} } // namespace hyrise::access

