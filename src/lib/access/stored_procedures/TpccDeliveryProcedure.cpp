// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccDeliveryProcedure.h"

#include <storage/AbstractTable.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/JoinScan.h>
#include <access/GroupByScan.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccDeliveryProcedure>("/TPCC-Delivery/");
} // namespace


TpccDeliveryProcedure::TpccDeliveryProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccDeliveryProcedure::setData(Json::Value& data) {
  //TODO
}

std::string TpccDeliveryProcedure::name() {
  return "TPCC-Delivery";
}

const std::string TpccDeliveryProcedure::vname() {
  return "TPCC-Delivery";
}

Json::Value TpccDeliveryProcedure::execute() {
  _tx = startTransaction();

  //TODO

  commit(_tx);

  // Output
  
  //TODO
}

}} // namespace hyrise::access

