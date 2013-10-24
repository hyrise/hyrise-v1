// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccDeliveryProcedure.h"

#include <storage/AbstractTable.h>
#include <access.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccDeliveryProcedure>("/TPCC-Delivery/");
} // namespace


TpccDeliveryProcedure::TpccDeliveryProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccDeliveryProcedure::setData(Json::Value& data) {
  _w_id = data["W_ID"].asInt();
  _d_id = data["D_ID"].asInt();
  _o_carrier_id = data["O_CARRIER_ID"].asInt();
}

std::string TpccDeliveryProcedure::name() {
  return "TPCC-Delivery";
}

const std::string TpccDeliveryProcedure::vname() {
  return "TPCC-Delivery";
}

Json::Value TpccDeliveryProcedure::execute() {
  _date = getDate();
  _tx = startTransaction();

  auto t1 = getNewOrder();
  _o_id = t1->getValue<int>("NO_O_ID", 0);

  auto t2 = getCId();
  _c_id = t2->getValue<int>("O_C_ID", 0);

  auto t3 = sumOLAmount();
  _total = t3->getValue<float>(0, 0);

  deleteNewOrder();
  updateOrders();
  updateOrderLine();
  updateCustomer();

  commit(_tx);

  Json::Value result;
  result["W_ID"] = _w_id;
  result["O_CARRIER_ID"] = _o_carrier_id;
  result["Execution Status"] = "Delivery has been queued";

  return result;
}

void TpccDeliveryProcedure::deleteNewOrder() {
  auto newOrder = getTpccTable("NEW_ORDER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(newOrder, "NO_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(newOrder, "NO_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(newOrder, "NO_O_ID", _o_id));
  auto validated = selectAndValidate(newOrder, connectAnd(expressions), _tx);

  deleteRows(validated, _tx);
}

storage::c_atable_ptr_t TpccDeliveryProcedure::getCId() {
  auto orders = getTpccTable("ORDERS", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orders, "O_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orders, "O_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orders, "O_ID", _o_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions), _tx);

  auto result = project(validated, {"O_C_ID"}, _tx);

  return result;
}

storage::c_atable_ptr_t TpccDeliveryProcedure::getNewOrder() {
  auto newOrder = getTpccTable("NEW_ORDER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(newOrder, "NO_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(newOrder, "NO_D_ID", _d_id));
  auto validated = selectAndValidate(newOrder, connectAnd(expressions), _tx);

  auto sorted = sort(validated, "NO_O_ID", true, _tx);

  auto result = project(sorted, {"NO_O_ID"}, _tx);

  return result;
}

storage::c_atable_ptr_t TpccDeliveryProcedure::sumOLAmount() {
  auto orderLine = getTpccTable("ORDER_LINE", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orderLine, "OL_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orderLine, "OL_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orderLine, "OL_O_ID", _o_id));
  auto validated = selectAndValidate(orderLine, connectAnd(expressions), _tx);

  GroupByScan groupby;
  groupby.addInput(validated);
  auto sum = new SumAggregateFun("OL_AMOUNT");
  groupby.addFunction(sum);
  groupby.execute();

  return groupby.getResultTable();
}

void TpccDeliveryProcedure::updateCustomer() {

  auto customer = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(customer, "C_ID", _c_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_D_ID", _d_id));
  auto validated = selectAndValidate(customer, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["C_BALANCE"] = _total;
  update(validated, updates, _tx);
}

void TpccDeliveryProcedure::updateOrderLine() {
  auto orderLine = getTpccTable("ORDER_LINE", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orderLine, "OL_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orderLine, "OL_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orderLine, "OL_O_ID", _o_id));
  auto validated = selectAndValidate(orderLine, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["OL_DELIVERY_D"] = _date;
  update(validated, updates, _tx);
}

void TpccDeliveryProcedure::updateOrders() {
  auto orders = getTpccTable("ORDERS", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orders, "O_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orders, "O_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orders, "O_ID", _o_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["O_CARRIER_ID"] = _o_carrier_id;
  update(validated, updates, _tx);
}


}} // namespace hyrise::access

