// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccDeliveryProcedure.h"

#include <storage/AbstractTable.h>
#include <access.h>
#include <unistd.h>
#include "access/CompoundIndexScan.h"

namespace hyrise {
namespace access {

namespace {
auto _ = net::Router::registerRoute<TpccDeliveryProcedure>("/TPCC-Delivery/");
}  // namespace


TpccDeliveryProcedure::TpccDeliveryProcedure(net::AbstractConnection* connection) : TpccStoredProcedure(connection) {}

void TpccDeliveryProcedure::setData(const Json::Value& data) {
  _w_id = assureMemberExists(data, "W_ID").asInt();
  _o_carrier_id = assureIntValueBetween(data, "O_CARRIER_ID", 1, 10);
}

std::string TpccDeliveryProcedure::name() { return "TPCC-Delivery"; }

const std::string TpccDeliveryProcedure::vname() { return "TPCC-Delivery"; }

Json::Value TpccDeliveryProcedure::execute() {
  _date = getDate();

  storage::c_atable_ptr_t tNewOrder;
  for (int i = 0; i < 10; i++) {

    startTransaction();

    _d_id = i + 1;
    tNewOrder = getNewOrder();

    if (tNewOrder->size() == 0)
      continue;
    _o_id = tNewOrder->getValue<hyrise_int_t>("NO_O_ID", 0);

    auto tOrder = getCId();
    if (tOrder->size() == 0) {
      throw std::runtime_error("internal error: new order is associated with non-existing order");
    }
    _c_id = tOrder->getValue<hyrise_int_t>("O_C_ID", 0);

    auto tSum = sumOLAmount();
    if (tSum->size() == 0) {
      throw std::runtime_error("internal error: no order lines for existing order");
    }
    _total = tSum->getValue<hyrise_float_t>(0, 0);

    deleteNewOrder();
    updateOrders(std::const_pointer_cast<storage::AbstractTable>(tOrder));
    updateOrderLine();
    updateCustomer();
  }

  // todo write output file?
  Json::Value result;
  result["W_ID"] = _w_id;
  result["O_CARRIER_ID"] = _o_carrier_id;
  result["Execution Status"] = "Delivery has been queued";

  return result;
}

void TpccDeliveryProcedure::deleteNewOrder() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("NEW_ORDER"));
  scan.setMainIndex("mcidx__NEW_ORDER__main__NO_W_ID__NO_D_ID__NO_O_ID");
  scan.setDeltaIndex("mcidx__NEW_ORDER__delta__NO_W_ID__NO_D_ID__NO_O_ID");
  scan.addPredicate<hyrise_int_t>("NO_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("NO_D_ID", _d_id);
  scan.addPredicate<hyrise_int_t>("NO_O_ID", _o_id);
  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  deleteRows(scan.getResultTable());
}

storage::c_atable_ptr_t TpccDeliveryProcedure::getCId() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("ORDERS"));
  scan.setMainIndex("mcidx__ORDERS__main__O_W_ID__O_D_ID__O_ID");
  scan.setDeltaIndex("mcidx__ORDERS__delta__O_W_ID__O_D_ID__O_ID");
  scan.addPredicate<hyrise_int_t>("O_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("O_D_ID", _d_id);
  scan.addPredicate<hyrise_int_t>("O_ID", _o_id);
  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  return scan.getResultTable();
}

storage::c_atable_ptr_t TpccDeliveryProcedure::getNewOrder() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("NEW_ORDER"));
  // scan.setMainIndex("mcidx__NEW_ORDER__main__NO_W_ID__NO_D_ID");
  // scan.setDeltaIndex("mcidx__NEW_ORDER__delta__NO_W_ID__NO_D_ID");
  // scan.addPredicate<hyrise_int_t>("NO_W_ID", _w_id);
  // scan.addPredicate<hyrise_int_t>("NO_D_ID", _d_id);

  scan.setMainIndex("mcidx__NEW_ORDER__main__NO_W_ID__NO_D_ID__NO_O_ID");
  scan.setDeltaIndex("mcidx__NEW_ORDER__delta__NO_W_ID__NO_D_ID__NO_O_ID");
  scan.addPredicate<hyrise_int_t>("NO_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("NO_D_ID", _d_id);


  scan.setValidation(true);
  scan.setTXContext(_tx);
  scan.execute();

  auto sorted = sort(scan.getResultTable(), "NO_O_ID", true);
  return sorted;
}

storage::c_atable_ptr_t TpccDeliveryProcedure::sumOLAmount() {
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

  std::shared_ptr<HashBuild> hash = std::make_shared<HashBuild>();
  hash->setOperatorId("__HashBuild");
  hash->setPlanOperationName("HashBuild");
  _responseTask->registerPlanOperation(hash);
  hash->addInput(scan.getResultTable());
  hash->addNamedField("OL_AMOUNT");
  hash->setKey("groupby");
  hash->execute();

  std::shared_ptr<GroupByScan> groupby = std::make_shared<GroupByScan>();
  groupby->setOperatorId("__GroupByScan");
  groupby->setPlanOperationName("GroupByScan");
  _responseTask->registerPlanOperation(groupby);

  groupby->addInput(scan.getResultTable());
  groupby->addInput(hash->getResultHashTable());
  auto sum = new SumAggregateFun("OL_AMOUNT");
  groupby->addFunction(sum);
  groupby->execute();

  return groupby->getResultTable();
}

void TpccDeliveryProcedure::updateCustomer() {
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

  Json::Value updates;
  updates["C_BALANCE"] = _total;
  update(scan.getResultTable(), updates);
}

void TpccDeliveryProcedure::updateOrderLine() {
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

  Json::Value updates;
  updates["OL_DELIVERY_D"] = _date;
  update(scan.getResultTable(), updates);
}

void TpccDeliveryProcedure::updateOrders(const storage::atable_ptr_t& ordersRow) {
  Json::Value updates;
  updates["O_CARRIER_ID"] = _o_carrier_id;
  update(ordersRow, updates);
}
}
}  // namespace hyrise::access
