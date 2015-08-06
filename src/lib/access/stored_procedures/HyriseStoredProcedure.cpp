// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "HyriseStoredProcedure.h"
#include "TpccDeliveryProcedure.h"
#include "TpccNewOrderProcedure.h"
#include "TpccOrderStatusProcedure.h"
#include "TpccPaymentProcedure.h"
#include "TpccStockLevelProcedure.h"

#include <helper/HttpHelper.h>


namespace hyrise {
namespace access {

namespace {
auto _ = net::Router::registerRoute<HyriseStoredProcedure>("/procedure/");
}  // namespace

HyriseStoredProcedure::HyriseStoredProcedure(net::AbstractConnection* connection) : _connection(connection) {}

std::string HyriseStoredProcedure::name() { return "HyriseStoredProcedure"; }

const std::string HyriseStoredProcedure::vname() { return "HyriseStoredProcedure"; }

void HyriseStoredProcedure::operator()() {

  if (!_connection->hasBody())
    throw std::runtime_error("message has no body");

  std::map<std::string, std::string> body_data = parseHTTPFormData(_connection->getBody());
  if (body_data.find("procedure") == body_data.end()) {
    throw std::runtime_error("No procedure specified");
  }
  if (body_data.find("query") == body_data.end()) {
    throw std::runtime_error("No data object in json");
  }

  auto request = urldecode(body_data["query"]);
  auto procedure = urldecode(body_data["procedure"]);

  Json::Value request_data;
  Json::Reader reader;

  if (!reader.parse(request, request_data)) {
    throw std::runtime_error("Failed to parse json");
  }

  if (procedure == "TPCC-NewOrder") {
    TpccNewOrderProcedure p(_connection);
    p();
  } else if (procedure == "TPCC-Payment") {
    TpccPaymentProcedure p(_connection);
    p();
  } else if (procedure == "TPCC-OrderStatus") {
    TpccOrderStatusProcedure p(_connection);
    p();
  } else if (procedure == "TPCC-Delivery") {
    TpccDeliveryProcedure p(_connection);
    p();
  } else if (procedure == "TPCC-StockLevel") {
    TpccStockLevelProcedure p(_connection);
    p();
  } else {
    throw std::runtime_error("Unknown stored procedure.");
  }
}
}
}  // namespace hyrise::access
