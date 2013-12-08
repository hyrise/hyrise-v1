// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/// This module implements a request parser registration facility for the ebb based server
/// implemented in `AsyncConnection.h`.

#ifndef SRC_LIB_NET_ROUTER_H_
#define SRC_LIB_NET_ROUTER_H_

#include <memory>
#include <unordered_map>
#include <string>

#include "net/AbstractConnection.h"
#include "taskscheduler/Task.h"

namespace hyrise {
namespace net {

/// Base class for request handlers
class AbstractRequestHandler : public taskscheduler::Task {
 public:
  typedef std::shared_ptr<AbstractRequestHandler> SharedPtr;
  virtual ~AbstractRequestHandler() {}
 protected:
  
};

struct AbstractRequestHandlerFactory {
  virtual AbstractRequestHandler::SharedPtr create(AbstractConnection *connection) const = 0;
  virtual ~AbstractRequestHandlerFactory() {}
};

/// Factory for request handlers, implements abstract factory pattern
template<typename T>
struct RequestHandlerFactory : public AbstractRequestHandlerFactory {
  AbstractRequestHandler::SharedPtr create(AbstractConnection *connection) const {
    return std::make_shared<T>(connection);
  }
};

/// For all routing related exceptions
class RouterException : public std::runtime_error {
 public:
  explicit RouterException(const std::string &message);
};

/// Central routing class, implements handling of requests mapped to registered
/// factories
class Router {
  friend class RouteRequestHandler;
 public:
  Router();
  Router(const Router &) = delete;
  Router& operator=(const Router&) = delete;

  typedef enum class DISPATCH { CATCH_ALL, EXACT } route_t;
  typedef std::unique_ptr<AbstractRequestHandlerFactory> handler_uptr;
  typedef const AbstractRequestHandlerFactory *handler_ptr;
  typedef std::unordered_map<std::string, handler_uptr> route_map_t;
  typedef std::unordered_map<std::string, std::string> route_name_t;

  /// Using an incoming url, returns a factory for the appropriate
  /// handler class, caller still has to call create to obtain their
  /// own handler
  /// @param[in] url URL
  static handler_ptr route(const std::string &url);

  /// Static initialization registration method for registering handler
  /// classes with urls, or to be more precisely, URL prefixes.
  /// @tparam RequestHandlerClass Class of request handler should inherit
  /// from AbstractRequestHandler
  /// @param[in] url URL to register
  /// @param[in] route optionally define route type
  ///
  ///   bool klass::registered = hyrise::net::Router::registerRoute<klass>("/url/")
  ///
  /// Registers `klass` to be a handler of all urls that start with "/url/"
  ///
  /// To avoid ordering problems with registrations, we disallow registration
  /// of "/", as this may result result in ambiguous resolution
  ///
  /// Instead, it is possible to register a CATCH_ALL handler that will be
  /// returned when no other handler has "claimed" a url-prefix.
  ///
  ///   bool klass::registered = hyrise::net::Router::registerRoute<klass>("/catch/",
  ///                                hyrise::net::Router::route_t::CATCH_ALL)
  ///
  /// This will register `klass` to be a handler of all urls that would otherwise
  /// not match any handlers at all.
  template<typename RequestHandlerClass>
  static bool registerRoute(const std::string &url,
                            route_t route = route_t::EXACT) {
    auto &router = Router::getInstance();
    handler_uptr factory(new RequestHandlerFactory<RequestHandlerClass>);
    router.addRoute(url, std::move(factory), route, RequestHandlerClass::name());
    return true;
  }

  /// The following functions should be assumed protected, as they are internal
  /// to the implementation of Router. Yet, for ease of testing, they are public.
  /// protected:

  /// Returns global Router instance
  static Router &getInstance();

  /// Returns routing map of registered classes
  const route_map_t &getRouters() const;

  /// Setter for catchAll
  void setCatchAll(handler_ptr handler);

  /// Getter for catchAll, exception when not set
  handler_ptr getCatchAll() const;

  /// Raw getter for catchAll, no exception when not set
  handler_ptr getCatchAllRaw() const;

  /// Return appropriate handler
  /// @param[in] url Url to resolve
  handler_ptr getHandler(const std::string &url) const;

  /// Return name of handler for route
  std::string getHandlerNameForRoute(const std::string &url) const;

  /// Add a new route
  /// @param[in] url url prefix
  /// @param[in] factory factory that returns RequestHandler
  /// @param[in] route route mode
  /// @param[in] name name of the registered handler
  void addRoute(const std::string &url,
                handler_uptr factory,
                route_t route,
                const std::string &name = "undefined");

 private:
  route_map_t _route;
  handler_ptr _catch_all;
  route_name_t _route_names;

};
}
}
#endif  // SRC_LIB_NET_ROUTER_H_
