// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccPaymentProcedure.h"

#include <storage/AbstractTable.h>
#include <access.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccPaymentProcedure>("/TPCC-Payment/");
} // namespace


TpccPaymentProcedure::TpccPaymentProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccPaymentProcedure::setData(Json::Value& data) {
  _w_id = data["W_ID"].asInt();
  _d_id = data["D_ID"].asInt();
  _c_w_id = data["C_W_ID"].asInt();
  _c_d_id = data["C_D_ID"].asInt();
  _h_amount = data["H_AMOUNT"].asFloat();

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

std::string TpccPaymentProcedure::name() {
  return "TPCC-Payment";
}

const std::string TpccPaymentProcedure::vname() {
  return "TPCC-Payment";
}

Json::Value TpccPaymentProcedure::execute() {
  _date = getDate();
  _tx = startTransaction();

  std::shared_ptr<const AbstractTable> tCustomer;
  if (_customerById) {
    tCustomer = std::const_pointer_cast<const AbstractTable>(getCustomerByCId());
    _chosenOne = 0;
    _c_last = tCustomer->getValue<std::string>("C_LAST", 0);
  }
  else {
    tCustomer = std::const_pointer_cast<const AbstractTable>(getCustomersByLastName());
    _chosenOne = (tCustomer->size() - 1) / 2 - 1; //really -1 again?;
    _c_id = tCustomer->getValue<int>("C_ID", _chosenOne);
  }

  const std::string c_credit = tCustomer->getValue<std::string>("C_CREDIT", _chosenOne);
  const bool bc_customer = (c_credit == "BC");
  _c_payment_cnt = tCustomer->getValue<int>("C_PAYMENT_CNT", _chosenOne) + 1;
  _c_balance = tCustomer->getValue<float>("C_BALANCE", _chosenOne) + _h_amount;
  _c_ytd_payment = tCustomer->getValue<float>("C_YTD_PAYMENT", _chosenOne) + _h_amount;
  _c_data = tCustomer->getValue<float>("C_DATA", _chosenOne);

  auto tWarehouse = getWarehouse();
  _w_ytd = tWarehouse->getValue<float>("W_YTD", 0) + _h_amount;
  const std::string w_name = tWarehouse->getValue<std::string>("W_NAME", 0);
  //setParameter(map, "w_ytd", w_ytd + h_amount);

  auto tDistrict = getDistrict();
  _d_ytd = tDistrict->getValue<float>("D_YTD", 0) + _h_amount;
  const std::string d_name = tDistrict->getValue<std::string>("D_NAME", 0);
  _h_data = w_name + "    " + d_name;

  updateWarehouseBalance();
  updateDistrictBalance();

  if (bc_customer)
    updateBCCustomer();
  else
    updateGCCustomer();

  insertHistory();


  commit(_tx);

  Json::Value result;
  result["W_ID"]          = _w_id;
  result["D_ID"]          = _d_id;
  result["C_ID"]          = _c_id;
  result["C_W_ID"]        = _c_w_id;
  result["C_D_ID"]        = _c_d_id;
  result["H_AMOUNT"]      = _h_amount;
  result["H_DATE"]        = _date;

  result["W_STREET_1"]    = tWarehouse->getValue<std::string>("W_STREET_1", 0);
  result["W_STREET_2"]    = tWarehouse->getValue<std::string>("W_STREET_2", 0);
  result["W_CITY"]        = tWarehouse->getValue<std::string>("W_CITY", 0);
  result["W_STATE"]       = tWarehouse->getValue<std::string>("W_STATE", 0);
  result["W_ZIP"]         = tWarehouse->getValue<std::string>("W_ZIP", 0);

  result["D_STREET_1"]    = tDistrict->getValue<std::string>("D_STREET_1", 0);
  result["D_STREET_2"]    = tDistrict->getValue<std::string>("D_STREET_2", 0);
  result["D_CITY"]        = tDistrict->getValue<std::string>("D_CITY", 0);
  result["D_STATE"]       = tDistrict->getValue<std::string>("D_STATE", 0);
  result["D_ZIP"]         = tDistrict->getValue<std::string>("D_ZIP", 0);

  result["C_FIRST"]       = tDistrict->getValue<std::string>("C_FIRST", _chosenOne);
  result["C_MIDDLE"]      = tDistrict->getValue<std::string>("C_MIDDLE", _chosenOne);
  result["C_LAST"]        = _c_last;
  result["C_STREET_1"]    = tDistrict->getValue<std::string>("C_STREET_1", _chosenOne);
  result["C_STREET_2"]    = tDistrict->getValue<std::string>("C_STREET_2", _chosenOne);
  result["C_CITY"]        = tDistrict->getValue<std::string>("C_CITY", _chosenOne);
  result["C_STATE"]       = tDistrict->getValue<std::string>("C_STATE", _chosenOne);
  result["C_ZIP"]         = tDistrict->getValue<std::string>("C_ZIP", _chosenOne);
  result["C_PHONE"]       = tDistrict->getValue<std::string>("C_PHONE", _chosenOne);
  result["C_SINCE"]       = tDistrict->getValue<std::string>("C_SINCE", _chosenOne);
  result["C_CREDIT"]      = c_credit;
  result["C_CREDIT_LIM"]  = tDistrict->getValue<std::string>("C_CREDIT_LIM", _chosenOne);
  result["C_DISCOUNT"]    = tDistrict->getValue<std::string>("C_DISCOUNT", _chosenOne);
  result["C_BALANCE"]     = _c_balance;

  //only if BC? don't know
  result["C_DATA"]        = _c_data;
  return result;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getCustomerByCId() {
  auto customer = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_ID", _c_id));
  auto validated = selectAndValidate(customer, connectAnd(expressions), _tx);

  auto result = project(validated, {"C_ID", "C_FIRST", "C_MIDDLE", "C_LAST", "C_STREET_1",
                                    "C_STREET_2", "C_CITY", "C_STATE", "C_ZIP", "C_PHONE",
                                    "C_SINCE", "C_CREDIT", "C_CREDIT_LIM", "C_DISCOUNT",
                                    "C_BALANCE", "C_YTD_PAYMENT", "C_PAYMENT_CNT", "C_DATA"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getCustomersByLastName() {
  auto customer = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<std::string>(customer, "C_LAST", _c_last));
  auto validated = selectAndValidate(customer, connectAnd(expressions), _tx);

  auto sorted = sort(validated, "C_FIRST", true, _tx);

  auto result = project(sorted, {"C_ID", "C_FIRST", "C_MIDDLE", "C_LAST", "C_STREET_1",
                                 "C_STREET_2", "C_CITY", "C_STATE", "C_ZIP", "C_PHONE",
                                 "C_SINCE", "C_CREDIT", "C_CREDIT_LIM", "C_DISCOUNT",
                                 "C_BALANCE", "C_YTD_PAYMENT", "C_PAYMENT_CNT", "C_DATA"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getDistrict() {
  auto district = getTpccTable("DISTRICT", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions), _tx);

  auto result = project(validated, {"D_NAME", "D_STREET_1", "D_STREET_2", "D_CITY",
                                    "D_STATE", "D_ZIP", "D_YTD"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getWarehouse() {
  auto warehouse = getTpccTable("WAREHOUSE", _tx);

  auto expr = new EqualsExpression<int>(warehouse, "W_ID", _w_id);
  auto validated = selectAndValidate(warehouse, expr, _tx);

  auto result = project(validated, {"W_NAME", "W_STREET_1", "W_STREET_2", "W_CITY",
                                    "W_STATE", "W_ZIP", "W_YTD"}, _tx);
  return result;
}

void TpccPaymentProcedure::insertHistory() {
  auto history = std::const_pointer_cast<AbstractTable>(getTpccTable("HISTORY", _tx));

  auto newRow = history->copy_structure(nullptr, false, 1);
  newRow->setValue<int>(0, 0, _c_id);
  newRow->setValue<int>(1, 0, _c_d_id);
  newRow->setValue<int>(2, 0, _c_w_id);
  newRow->setValue<int>(3, 0, _d_id);
  newRow->setValue<int>(4, 0, _w_id);
  newRow->setValue<std::string>(5, 0, _date);
  newRow->setValue<float>(6, 0, _h_amount);
  newRow->setValue<std::string>(7, 0, _h_data);
  
  insert(history, newRow, _tx);
}

void TpccPaymentProcedure::updateDistrictBalance() {
  auto orders = getTpccTable("DISTRICT", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orders, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orders, "D_ID", _d_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["D_YTD"] = _d_ytd;
  update(validated, updates, _tx);
}

void TpccPaymentProcedure::updateBCCustomer() {
  auto orders = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orders, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orders, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orders, "C_ID", _c_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["C_BALANCE"] = _c_balance;
  updates["C_YTD_PAYMENT"] = _c_ytd_payment;
  updates["C_PAYMENT_CNT"] = _c_payment_cnt;
  updates["C_DATA"] = _c_data;
  update(validated, updates, _tx);
}

void TpccPaymentProcedure::updateGCCustomer() {
  auto orders = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(orders, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(orders, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(orders, "C_ID", _c_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["C_BALANCE"] = _c_balance;
  updates["C_YTD_PAYMENT"] = _c_ytd_payment;
  updates["C_PAYMENT_CNT"] = _c_payment_cnt;
  update(validated, updates, _tx);
}

void TpccPaymentProcedure::updateWarehouseBalance() {
  auto orders = getTpccTable("WAREHOUSE", _tx);

  auto expr = new EqualsExpression<int>(orders, "W_ID", _w_id);
  auto validated = selectAndValidate(orders, expr, _tx);

  Json::Value updates;
  updates["W_YTD"] = _w_ytd;
  update(validated, updates, _tx);
}

} } // namespace hyrise::access

