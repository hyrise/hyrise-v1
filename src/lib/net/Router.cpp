// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "net/Router.h"

#include <iostream>
#include <string>

namespace hyrise {
namespace net {

RouterException::RouterException(const std::string& message) : std::runtime_error(message) {}

Router::Router() : _catch_all(nullptr) {}

Router& Router::getInstance() {
  static Router r;
  return r;
}

const Router::route_map_t& Router::getRouters() const { return _route; }

void Router::addRoute(const std::string& url, handler_uptr handler, route_t route, const std::string& handler_name) {
  if (url == "/")
    throw RouterException(
        "Registering for '/' is not possible, use "
        "Router::route_t::CATCH_ALL to register "
        "for all unmatched urls");
  _route[url].swap(handler);
  _route_names[url] = handler_name;
  if (route == route_t::CATCH_ALL)
    setCatchAll(_route[url].get());
}

void Router::setCatchAll(const handler_ptr handler) {
  if (_catch_all != nullptr)
    throw RouterException("Tried to set catch all when already set");
  _catch_all = handler;
}

Router::handler_ptr Router::getCatchAll() const {
  if (_catch_all == nullptr)
    throw RouterException("No url was set with Router::route_t::CATCH_ALL");
  return _catch_all;
}

Router::handler_ptr Router::getCatchAllRaw() const { return _catch_all; }

std::string Router::getHandlerNameForRoute(const std::string& url) const { return _route_names.at(url); }

Router::handler_ptr Router::getHandler(const std::string& url) const {
  for (const auto& kv : _route) {
    auto route_url = kv.first;
    if (url.find(route_url) != std::string::npos) {
      return kv.second.get();
    }
  }
  return getCatchAll();
}

Router::handler_ptr Router::route(const std::string& url) {
  const auto& router = getInstance();
  return router.getHandler(url);
}
}
}
