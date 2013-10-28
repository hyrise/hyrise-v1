// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "net/RouteRequestHandler.h"

#include <sstream>
#include <string>
#include <iostream>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "net/AsyncConnection.h"

namespace hyrise {
namespace net {

bool RouteRequestHandler::registered =
    Router::registerRoute<RouteRequestHandler>("/urls/");

RouteRequestHandler::RouteRequestHandler(AbstractConnection *data)
    : _connection_data(data) {}

std::string RouteRequestHandler::name() {
  return "RouteRequestHandler";
}

const std::string RouteRequestHandler::vname() {
  return "RouteRequestHandler";
}

std::string RouteRequestHandler::constructResponse() {
  const auto &router = Router::getInstance();
  rapidjson::Document result;
  result.SetObject();

  rapidjson::Value routes(rapidjson::kArrayType);
  auto& allocator = result.GetAllocator();

  const auto& catch_all = router.getCatchAllRaw();
  for (const auto & route: router.getRouters()) {


    rapidjson::Value entry(rapidjson::kObjectType);
    entry.AddMember("handler", router.getHandlerNameForRoute(route.first).c_str(), allocator);

    if (route.second.get() == catch_all)
      entry.AddMember("catch_all", true, allocator);
    routes.PushBack(entry, allocator);
  }
 
  result.AddMember("routes", routes, allocator);

  rapidjson::StringBuffer wss;
  rapidjson::Writer<decltype(wss)> w(wss);
  result.Accept(w);

  return wss.GetString();
}

void RouteRequestHandler::operator()() {
  std::string response(constructResponse());
  _connection_data->respond(response);
}
}
}
