// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccOrderStatusProcedure.h"

#include <storage/AbstractTable.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/JoinScan.h>
#include <access/GroupByScan.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccOrderStatusProcedure>("/TPCC-OrderStatus/");
} // namespace


TpccOrderStatusProcedure::TpccOrderStatusProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccOrderStatusProcedure::setData(Json::Value& data) {
  //TODO
}

std::string TpccOrderStatusProcedure::name() {
  return "TPCC-OrderStatus";
}

const std::string TpccOrderStatusProcedure::vname() {
  return "TPCC-OrderStatus";
}

Json::Value TpccOrderStatusProcedure::execute() {
  // Transaction
  _tx = startTransaction();

  //TODO

  commit(_tx);

  // Output
  //TODO
}

} } // namespace hyrise::access

