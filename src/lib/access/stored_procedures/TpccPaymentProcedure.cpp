// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccPaymentProcedure.h"

#include <storage/AbstractTable.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/JoinScan.h>
#include <access/GroupByScan.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccPaymentProcedure>("/TPCC-Payment/");
} // namespace


TpccPaymentProcedure::TpccPaymentProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccPaymentProcedure::setData(Json::Value& data) {
  //TODO
}

std::string TpccPaymentProcedure::name() {
  return "TPCC-Payment";
}

const std::string TpccPaymentProcedure::vname() {
  return "TPCC-Payment";
}

Json::Value TpccPaymentProcedure::execute() {
  // Transaction
  _tx = startTransaction();

  //TODO

  commit(_tx);

  // Output
  //TODO
}

} } // namespace hyrise::access

