// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "net/RouteRequestHandler.h"

#include <sstream>
#include <string>

#include "json.h"
#include "net/AsyncConnection.h"

namespace hyrise {
namespace net {

bool RouteRequestHandler::registered = Router::registerRoute<RouteRequestHandler>("/urls/");

RouteRequestHandler::RouteRequestHandler(AbstractConnection* data) : _connection_data(data) {}

std::string RouteRequestHandler::name() { return "RouteRequestHandler"; }

const std::string RouteRequestHandler::vname() { return "RouteRequestHandler"; }

std::string RouteRequestHandler::constructResponse() {
  const auto& router = Router::getInstance();
  Json::Value result;
  auto& routes = result["routes"];
  const auto& catch_all = router.getCatchAllRaw();
  for (const auto& route : router.getRouters()) {
    routes[route.first]["handler"] = router.getHandlerNameForRoute(route.first);
    if (route.second.get() == catch_all)
      routes[route.first]["catch_all"] = true;
  }
  Json::StyledWriter writer;
  return writer.write(result);
}

void RouteRequestHandler::operator()() {
  std::string response(constructResponse());
  _connection_data->respond(response);
}
}
}
