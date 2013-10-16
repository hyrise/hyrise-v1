// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "net/StoredProcedureHandler.h"

#include <sstream>
#include <string>

#include "json.h"
#include "net/AsyncConnection.h"

namespace hyrise {
namespace net {

bool StoredProcedureHandler::registered =
    Router::registerRoute<StoredProcedureHandler>("/stored_procedure/");

StoredProcedureHandler::StoredProcedureHandler(AbstractConnection *data)
    : _connection_data(data) {}

std::string StoredProcedureHandler::name() {
  return "StoredProcedureHandler";
}

const std::string StoredProcedureHandler::vname() {
  return "StoredProcedureHandler";
}

std::string StoredProcedureHandler::constructResponse() {
  const auto &router = Router::getInstance();
  Json::Value result;
  result["Hallo"] = "Welt";
  Json::StyledWriter writer;
  return writer.write(result);
}

void StoredProcedureHandler::operator()() {
  std::string response(constructResponse());
  _connection_data->respond(response);
}
}
}
