// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccOrderStatusProcedure.h"

#include <storage/AbstractTable.h>
#include <access.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccOrderStatusProcedure>("/TPCC-OrderStatus/");
} // namespace


TpccOrderStatusProcedure::TpccOrderStatusProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccOrderStatusProcedure::setData(Json::Value& data) {
  _w_id = data["W_ID"].asInt();
  _d_id = data["D_ID"].asInt();

  const int id = data["C_ID"].isInt();
  const bool last = data["C_LAST"].isString();
  if (id && last)
    throw std::runtime_error("Customer selection via either C_ID or C_LAST - both are provided");
  else if (!id && !last)
    throw std::runtime_error("Customer selection via either C_ID or C_LAST - provide one of them");
  else if (id) {
    _customerById = true;
    _c_id = data["C_ID"].asInt();
  }
  else {
    _customerById = false;
    _c_last = data["C_LAST"].asString();
  }
}

std::string TpccOrderStatusProcedure::name() {
  return "TPCC-OrderStatus";
}

const std::string TpccOrderStatusProcedure::vname() {
  return "TPCC-OrderStatus";
}

Json::Value TpccOrderStatusProcedure::execute() {
  _tx = startTransaction();

  storage::atable_ptr_t tCustomer;
  if (!_customerById) {
    tCustomer = std::const_pointer_cast<AbstractTable>(getCustomerByCId());
    _chosenOne = 0;
    _c_last = tCustomer->getValue<std::string>("C_LAST", 0);
  }
  else {
    tCustomer = std::const_pointer_cast<AbstractTable>(getCustomersByLastName());
    //TODO what if size = 0?
    _chosenOne = (tCustomer->size() - 1) / 2;
    _c_id = tCustomer->getValue<int>("C_ID", _chosenOne);
  }

  auto t2 = getLastOrder();
  _o_id = t2->getValue<int>("O_ID", 0);
  const std::string o_entry_d = t2->getValue<std::string>("O_ENTRY_D", 0);
  const int o_carrier_id = t2->getValue<int>("O_CARRIER_ID", 0);

  auto tOrderLines = getOrderLines();

  commit(_tx);

  // Output
  Json::Value result;
  result["W_ID"]         = _w_id;
  result["D_ID"]         = _d_id;
  result["C_ID"]         = _c_id;
  result["C_FIRST"]      = tCustomer->getValue<std::string>("C_FIRST", _chosenOne);
  result["C_MIDDLE"]     = tCustomer->getValue<std::string>("C_MIDDLE", _chosenOne);
  result["C_LAST"]       = _c_last;
  result["C_BALANCE"]    = tCustomer->getValue<float>("C_BALANCE", _chosenOne);
  result["O_ID"]         = _o_id;
  result["O_ENTRY_D"]    = _date;
  result["O_CARRIER_ID"] = o_carrier_id;

  Json::Value orderLines(Json::arrayValue);
  for (size_t i = 0; i < tOrderLines->size(); ++i) {
    Json::Value orderLine;
    orderLine["OL_SUPPLY_W_ID"] = tOrderLines->getValue<int>("OL_SUPPLY_W_ID", i);
    orderLine["OL_I_ID"]        = tOrderLines->getValue<int>("OL_I_ID", i);
    orderLine["OL_QUANTITY"]    = tOrderLines->getValue<int>("OL_QUANTITY", i);
    orderLine["OL_AMOUNT"]      = tOrderLines->getValue<float>("OL_AMOUNT", i);
    orderLine["OL_DELIVERY_D"]  = tOrderLines->getValue<std::string>("OL_DELIVERY_D", i);
    orderLines.append(orderLine);
  }
  result["order lines"] = orderLines;

  return result;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getCustomerByCId() {
  auto customer = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_ID", _c_id));
  auto validated = selectAndValidate(customer, connectAnd(expressions), _tx);

  auto result = project(validated, {"C_ID", "C_FIRST", "C_MIDDLE", "C_LAST",
                                    "C_BALANCE"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getCustomersByLastName() {
  auto customer = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<std::string>(customer, "C_LAST", _c_last));
  auto validated = selectAndValidate(customer, connectAnd(expressions), _tx);

  auto sorted = sort(validated, "C_FIRST", true, _tx);

  auto result = project(sorted, {"C_ID", "C_FIRST", "C_MIDDLE", "C_LAST", "C_BALANCE"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getLastOrder() {
  auto orders = getTpccTable("ORDERS", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orders, "O_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orders, "O_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orders, "O_C_ID", _d_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions), _tx);

  auto sorted = sort(validated, "O_ID", true, _tx);

  auto result = project(sorted, {"O_ID", "O_CARRIER_ID", "O_ENTRY_D"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getOrderLines() {
  auto order_line = getTpccTable("ORDER_LINE", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(order_line, "OL_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(order_line, "OL_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(order_line, "OL_O_ID", _o_id));
  auto validated = selectAndValidate(order_line, connectAnd(expressions), _tx);

  auto result = project(validated, {"OL_SUPPLY_W_ID", "OL_I_ID", "OL_QUANTITY",
                                    "OL_AMOUNT", "OL_DELIVERY_D"}, _tx);
  return result;
}

} } // namespace hyrise::access

