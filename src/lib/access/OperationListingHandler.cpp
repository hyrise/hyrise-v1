#include "access/OperationListingHandler.h"

#include "json.h"
#include "net/AsyncConnection.h"
#include "net/AbstractConnection.h"
#include "access/system/QueryParser.h"

namespace hyrise {
namespace access {

bool OperationListingHandler::registered = net::Router::registerRoute<OperationListingHandler>("/operations/");

OperationListingHandler::OperationListingHandler(net::AbstractConnection* data) : _connection_data(data) {}

std::string OperationListingHandler::name() { return "OperationListingHandler"; }

const std::string OperationListingHandler::vname() { return "OperationListingHandler"; }

std::string OperationListingHandler::constructResponse() {
  auto& qp = QueryParser::instance();
  Json::Value result;
  for (const auto& name : qp.getOperationNames()) {
    result[name] = Json::Value(Json::objectValue);
  }
  Json::StyledWriter writer;
  return writer.write(result);
}

void OperationListingHandler::operator()() {
  std::string response(constructResponse());
  _connection_data->respond(response);
}
}
}
