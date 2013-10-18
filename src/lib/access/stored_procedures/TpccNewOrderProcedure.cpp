// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccNewOrderProcedure.h"

#include <storage/AbstractTable.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/JoinScan.h>
#include <access/GroupByScan.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccNewOrderProcedure>("/TPCC-NewOrder/");
} // namespace


TpccNewOrderProcedure::TpccNewOrderProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccNewOrderProcedure::setData(Json::Value& data) {
  _w_id = data["W_ID"].asInt();
  _d_id = data["D_ID"].asInt();
  _threshold = data["threshold"].asInt();
}

std::string TpccNewOrderProcedure::name() {
  return "TPCC-NewOrder";
}

const std::string TpccNewOrderProcedure::vname() {
  return "TPCC-NewOrder";
}

Json::Value TpccNewOrderProcedure::execute() {
  // Transaction
  _tx = startTransaction();

  //TODO

  commit(_tx);

  // Output
  //TODO
}

} } // namespace hyrise::access

