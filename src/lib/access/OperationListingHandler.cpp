#include "access/OperationListingHandler.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


#include "net/AsyncConnection.h"
#include "net/AbstractConnection.h"
#include "access/system/QueryParser.h"

namespace hyrise {
namespace access {

bool OperationListingHandler::registered =
    net::Router::registerRoute<OperationListingHandler>("/operations/");

OperationListingHandler::OperationListingHandler(net::AbstractConnection *data)
    : _connection_data(data) {}

std::string OperationListingHandler::name() {
  return "OperationListingHandler";
}

const std::string OperationListingHandler::vname() {
  return "OperationListingHandler";
}

std::string OperationListingHandler::constructResponse() {
  auto &qp = QueryParser::instance();
  rapidjson::Document result;
  for (const auto & name: qp.getOperationNames()) {
    rapidjson::Value empty(rapidjson::kObjectType);
    result.AddMember(name.c_str(), empty, result.GetAllocator());
  }

  rapidjson::StringBuffer buf;
  rapidjson::Writer<decltype(buf)> writer(buf);
  result.Accept(writer);
  return buf.GetString();

}

void OperationListingHandler::operator()() {
  std::string response(constructResponse());
  _connection_data->respond(response);
}
}
}
