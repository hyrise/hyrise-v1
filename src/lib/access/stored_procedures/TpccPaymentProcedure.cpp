// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccPaymentProcedure.h"

#include <storage/AbstractTable.h>
#include <storage/TableBuilder.h>
#include <storage/storage_types_helper.h>
#include <access.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccPaymentProcedure>("/TPCC-Payment/");
} // namespace


TpccPaymentProcedure::TpccPaymentProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccPaymentProcedure::setData(Json::Value& data) {
  _w_id =     assureMemberExists(data, "W_ID").asInt();
  _d_id =     assureMemberExists(data, "D_ID").asInt();
  _c_w_id =   assureMemberExists(data, "C_W_ID").asInt();
  _c_d_id =   assureMemberExists(data, "C_D_ID").asInt();
  _h_amount = assureMemberExists(data, "H_AMOUNT").asFloat();

  const bool id = data["C_ID"].isInt();
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

  std::shared_ptr<const AbstractTable> tCustomer;
  if (_customerById) {
    tCustomer = std::const_pointer_cast<const AbstractTable>(getCustomerByCId());
    //TODO no customer
    _chosenOne = 0;
    _c_last = tCustomer->getValue<hyrise_string_t>("C_LAST", 0);
  }
  else {
    tCustomer = std::const_pointer_cast<const AbstractTable>(getCustomersByLastName());
    //TODO no customer
    _chosenOne = std::max<int>((tCustomer->size() - 1) / 2 - 1, 0); //really -1 again?;
    _c_id = tCustomer->getValue<int>("C_ID", _chosenOne);
  }

  const std::string c_credit = tCustomer->getValue<hyrise_string_t>("C_CREDIT", _chosenOne);
  const bool bc_customer = (c_credit == "BC");
  _c_payment_cnt = tCustomer->getValue<int>("C_PAYMENT_CNT", _chosenOne) + 1;
  _c_balance = tCustomer->getValue<float>("C_BALANCE", _chosenOne) + _h_amount;
  _c_ytd_payment = tCustomer->getValue<float>("C_YTD_PAYMENT", _chosenOne) + _h_amount;
  _c_data = tCustomer->getValue<hyrise_string_t>("C_DATA", _chosenOne);

  auto tWarehouse = getWarehouse();
  //TODO no warehouse
  _w_ytd = tWarehouse->getValue<float>("W_YTD", 0) + _h_amount;
  const std::string w_name = tWarehouse->getValue<hyrise_string_t>("W_NAME", 0);
  //setParameter(map, "w_ytd", w_ytd + h_amount);

  auto tDistrict = getDistrict();
  //TODO no district
  _d_ytd = tDistrict->getValue<float>("D_YTD", 0) + _h_amount;
  const std::string d_name = tDistrict->getValue<hyrise_string_t>("D_NAME", 0);
  _h_data = w_name + "    " + d_name;

  updateWarehouseBalance();
  updateDistrictBalance();

  if (bc_customer)
    updateBCCustomer();
  else
    updateGCCustomer();

  insertHistory();


  commit();

  Json::Value result;
  result["W_ID"]          = _w_id;
  result["D_ID"]          = _d_id;
  result["C_ID"]          = _c_id;
  result["C_W_ID"]        = _c_w_id;
  result["C_D_ID"]        = _c_d_id;
  result["H_AMOUNT"]      = _h_amount;
  result["H_DATE"]        = _date;

  result["W_STREET_1"]    = tWarehouse->getValue<hyrise_string_t>("W_STREET_1", 0);
  result["W_STREET_2"]    = tWarehouse->getValue<hyrise_string_t>("W_STREET_2", 0);
  result["W_CITY"]        = tWarehouse->getValue<hyrise_string_t>("W_CITY", 0);
  result["W_STATE"]       = tWarehouse->getValue<hyrise_string_t>("W_STATE", 0);
  result["W_ZIP"]         = tWarehouse->getValue<hyrise_string_t>("W_ZIP", 0);

  result["D_STREET_1"]    = tDistrict->getValue<hyrise_string_t>("D_STREET_1", 0);
  result["D_STREET_2"]    = tDistrict->getValue<hyrise_string_t>("D_STREET_2", 0);
  result["D_CITY"]        = tDistrict->getValue<hyrise_string_t>("D_CITY", 0);
  result["D_STATE"]       = tDistrict->getValue<hyrise_string_t>("D_STATE", 0);
  result["D_ZIP"]         = tDistrict->getValue<hyrise_string_t>("D_ZIP", 0);

  result["C_FIRST"]       = tCustomer->getValue<hyrise_string_t>("C_FIRST", _chosenOne);
  result["C_MIDDLE"]      = tCustomer->getValue<hyrise_string_t>("C_MIDDLE", _chosenOne);
  result["C_LAST"]        = _c_last;
  result["C_STREET_1"]    = tCustomer->getValue<hyrise_string_t>("C_STREET_1", _chosenOne);
  result["C_STREET_2"]    = tCustomer->getValue<hyrise_string_t>("C_STREET_2", _chosenOne);
  result["C_CITY"]        = tCustomer->getValue<hyrise_string_t>("C_CITY", _chosenOne);
  result["C_STATE"]       = tCustomer->getValue<hyrise_string_t>("C_STATE", _chosenOne);
  result["C_ZIP"]         = tCustomer->getValue<hyrise_string_t>("C_ZIP", _chosenOne);
  result["C_PHONE"]       = tCustomer->getValue<hyrise_string_t>("C_PHONE", _chosenOne);
  result["C_SINCE"]       = tCustomer->getValue<hyrise_string_t>("C_SINCE", _chosenOne);
  result["C_CREDIT"]      = c_credit;
  result["C_CREDIT_LIM"]  = tCustomer->getValue<hyrise_float_t>("C_CREDIT_LIM", _chosenOne);
  result["C_DISCOUNT"]    = tCustomer->getValue<hyrise_float_t>("C_DISCOUNT", _chosenOne);
  result["C_BALANCE"]     = _c_balance;

  //only if BC? don't know
  result["C_DATA"]        = _c_data;
  return result;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getCustomerByCId() {
  auto customer = getTpccTable("CUSTOMER");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_ID", _c_id));
  auto validated = selectAndValidate(customer, connectAnd(expressions));

  return validated;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getCustomersByLastName() {
  auto customer = getTpccTable("CUSTOMER");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<hyrise_string_t>(customer, "C_LAST", _c_last));
  auto validated = selectAndValidate(customer, connectAnd(expressions));

  auto sorted = sort(validated, "C_FIRST", true);
  return sorted;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getDistrict() {
  auto district = getTpccTable("DISTRICT");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions));

  return validated;
}

storage::c_atable_ptr_t TpccPaymentProcedure::getWarehouse() {
  auto warehouse = getTpccTable("WAREHOUSE");

  auto expr = new EqualsExpression<hyrise_int_t>(warehouse, "W_ID", _w_id);
  auto validated = selectAndValidate(warehouse, expr);

  return validated;
}

void TpccPaymentProcedure::insertHistory() {
  auto history = std::const_pointer_cast<AbstractTable>(getTpccTable("HISTORY"));

  const auto metadata = history->metadata();
  storage::TableBuilder::param_list list;
  for (const auto& columnData : metadata) {
    list.append(storage::TableBuilder::param(columnData.getName(), data_type_to_string(columnData.getType())));
  }
  //auto newRow = history->copy_structure(nullptr, false, 1);//storage::TableBuilder::build(list);
  auto newRow = storage::TableBuilder::build(list);
  newRow->resize(400);
  newRow->setValue<hyrise_int_t>("H_C_ID", 0, _c_id);
  newRow->setValue<hyrise_int_t>("H_C_D_ID", 0, _c_d_id);
  newRow->setValue<hyrise_int_t>(2, 0, _c_w_id);
  newRow->setValue<hyrise_int_t>(3, 0, _d_id);
  newRow->setValue<hyrise_int_t>(4, 0, _w_id);
  newRow->setValue<hyrise_string_t>(5, 0, _date);
  newRow->setValue<hyrise_float_t>(6, 0, _h_amount);
  newRow->setValue<hyrise_string_t>(7, 0, _h_data);
  
  //insert(history, newRow);
}

void TpccPaymentProcedure::updateDistrictBalance() {
  auto orders = getTpccTable("DISTRICT");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "D_ID", _d_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions));

  Json::Value updates;
  updates["D_YTD"] = _d_ytd;
  update(validated, updates);
}

void TpccPaymentProcedure::updateBCCustomer() {
  auto orders = getTpccTable("CUSTOMER");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "C_ID", _c_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions));

  Json::Value updates;
  updates["C_BALANCE"] = _c_balance;
  updates["C_YTD_PAYMENT"] = _c_ytd_payment;
  updates["C_PAYMENT_CNT"] = _c_payment_cnt;
  updates["C_DATA"] = _c_data;
  update(validated, updates);
}

void TpccPaymentProcedure::updateGCCustomer() {
  auto orders = getTpccTable("CUSTOMER");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(orders, "C_ID", _c_id));
  auto validated = selectAndValidate(orders, connectAnd(expressions));

  Json::Value updates;
  updates["C_BALANCE"] = _c_balance;
  updates["C_YTD_PAYMENT"] = _c_ytd_payment;
  updates["C_PAYMENT_CNT"] = _c_payment_cnt;
  update(validated, updates);
}

void TpccPaymentProcedure::updateWarehouseBalance() {
  auto orders = getTpccTable("WAREHOUSE");

  auto expr = new EqualsExpression<hyrise_int_t>(orders, "W_ID", _w_id);
  auto validated = selectAndValidate(orders, expr);

  Json::Value updates;
  updates["W_YTD"] = _w_ytd;
  update(validated, updates);
}

} } // namespace hyrise::access

