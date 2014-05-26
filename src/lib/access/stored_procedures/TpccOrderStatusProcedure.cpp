// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccOrderStatusProcedure.h"

#include <storage/AbstractTable.h>

#include "access/CompoundIndexScan.h"

namespace hyrise {
namespace access {

namespace {
auto _ = net::Router::registerRoute<TpccOrderStatusProcedure>("/TPCC-OrderStatus/");
}  // namespace


TpccOrderStatusProcedure::TpccOrderStatusProcedure(net::AbstractConnection* connection)
    : TpccStoredProcedure(connection) {}

void TpccOrderStatusProcedure::setData(const Json::Value& data) {
  _w_id = assureMemberExists(data, "W_ID").asInt();
  _d_id = assureIntValueBetween(data, "D_ID", 1, 10);

  const bool id = data["C_ID"].isInt();
  const bool last = data["C_LAST"].isString();
  if (id && last)
    throw std::runtime_error("Customer selection via either C_ID or C_LAST - both are provided");
  else if (!id && !last)
    throw std::runtime_error("Customer selection via either C_ID or C_LAST - provide one of them");
  else if (id) {
    _customerById = true;
    _c_id = data["C_ID"].asInt();
  } else {
    _customerById = false;
    _c_last = data["C_LAST"].asString();
  }
}

std::string TpccOrderStatusProcedure::name() { return "TPCC-OrderStatus"; }

const std::string TpccOrderStatusProcedure::vname() { return "TPCC-OrderStatus"; }

Json::Value TpccOrderStatusProcedure::execute() {
  storage::atable_ptr_t tCustomer;
  if (_customerById) {
    tCustomer = std::const_pointer_cast<storage::AbstractTable>(getCustomerByCId());
    if (tCustomer->size() == 0) {
      std::ostringstream os;
      os << "No Customer with ID: " << _c_id;
      throw std::runtime_error(os.str());
    }

    _chosenOne = 0;
    _c_last = tCustomer->getValue<hyrise_string_t>("C_LAST", 0);
  } else {
    tCustomer = std::const_pointer_cast<storage::AbstractTable>(getCustomersByLastName());
    if (tCustomer->size() == 0) {
      throw std::runtime_error("No Customer with ID: " + _c_last);
    }

    _chosenOne = tCustomer->size() / 2;
    _c_id = tCustomer->getValue<hyrise_int_t>("C_ID", _chosenOne);
  }

  auto tOrders = getLastOrder();
  if (tOrders->size() == 0) {
    std::ostringstream os;
    os << "no active order for customer " << _c_id << " in district " << _d_id << " of warehouse " << _w_id;
    throw std::runtime_error(os.str());
  }
  _o_id = tOrders->getValue<hyrise_int_t>("O_ID", 0);
  const std::string o_entry_d = tOrders->getValue<hyrise_string_t>("O_ENTRY_D", 0);
  const int o_carrier_id = tOrders->getValue<hyrise_int_t>("O_CARRIER_ID", 0);

  auto tOrderLines = getOrderLines();
  if (tOrderLines->size() < 5 || tOrderLines->size() > 15)
    throw std::runtime_error("internal error: there must be between 5 and 15 orderlines for every order");

  // commit();

  // Output
  Json::Value result;
  result["W_ID"] = _w_id;
  result["D_ID"] = _d_id;
  result["C_ID"] = _c_id;
  result["C_FIRST"] = tCustomer->getValue<hyrise_string_t>("C_FIRST", _chosenOne);
  result["C_MIDDLE"] = tCustomer->getValue<hyrise_string_t>("C_MIDDLE", _chosenOne);
  result["C_LAST"] = _c_last;
  result["C_BALANCE"] = tCustomer->getValue<hyrise_float_t>("C_BALANCE", _chosenOne);
  result["O_ID"] = _o_id;
  result["O_ENTRY_D"] = _date;
  result["O_CARRIER_ID"] = o_carrier_id;

  Json::Value orderLines(Json::arrayValue);
  for (size_t i = 0; i < tOrderLines->size(); ++i) {
    Json::Value orderLine;
    orderLine["OL_SUPPLY_W_ID"] = tOrderLines->getValue<hyrise_int_t>("OL_SUPPLY_W_ID", i);
    orderLine["OL_I_ID"] = tOrderLines->getValue<hyrise_int_t>("OL_I_ID", i);
    orderLine["OL_QUANTITY"] = tOrderLines->getValue<hyrise_int_t>("OL_QUANTITY", i);
    orderLine["OL_AMOUNT"] = tOrderLines->getValue<hyrise_float_t>("OL_AMOUNT", i);
    orderLine["OL_DELIVERY_D"] = tOrderLines->getValue<hyrise_string_t>("OL_DELIVERY_D", i);
    orderLines.append(orderLine);
  }
  result["order lines"] = orderLines;

  return result;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getCustomerByCId() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("CUSTOMER"));
  scan.setMainIndex("mcidx__CUSTOMER__main__C_W_ID__C_ID__C_D_ID");
  scan.setDeltaIndex("mcidx__CUSTOMER__delta__C_W_ID__C_ID__C_D_ID");
  scan.addPredicate<hyrise_int_t>("C_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("C_ID", _c_id);
  scan.addPredicate<hyrise_int_t>("C_D_ID", _d_id);
  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  return scan.getResultTable();
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getCustomersByLastName() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("CUSTOMER"));
  scan.setMainIndex("mcidx__CUSTOMER__main__C_W_ID__C_D_ID__C_LAST");
  scan.setDeltaIndex("mcidx__CUSTOMER__delta__C_W_ID__C_D_ID__C_LAST");
  scan.addPredicate<hyrise_int_t>("C_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("C_D_ID", _d_id);
  scan.addPredicate<hyrise_string_t>("C_LAST", _c_last);
  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  auto sorted = sort(scan.getResultTable(), "C_FIRST", true);
  return sorted;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getLastOrder() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("ORDERS"));
  scan.setMainIndex("mcidx__ORDERS__main__O_W_ID__O_D_ID__O_C_ID");
  scan.setDeltaIndex("mcidx__ORDERS__delta__O_W_ID__O_D_ID__O_C_ID");
  scan.addPredicate<hyrise_int_t>("O_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("O_D_ID", _d_id);
  scan.addPredicate<hyrise_int_t>("O_C_ID", _c_id);
  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  auto sorted = sort(scan.getResultTable(), "O_ID", false);
  return sorted;
}

storage::c_atable_ptr_t TpccOrderStatusProcedure::getOrderLines() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("ORDER_LINE"));
  scan.setMainIndex("mcidx__ORDER_LINE__main__OL_W_ID__OL_D_ID__OL_O_ID");
  scan.setDeltaIndex("mcidx__ORDER_LINE__delta__OL_W_ID__OL_D_ID__OL_O_ID");
  scan.addPredicate<hyrise_int_t>("OL_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("OL_D_ID", _d_id);
  scan.addPredicate<hyrise_int_t>("OL_O_ID", _o_id);
  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  return scan.getResultTable();
}
}
}  // namespace hyrise::access
