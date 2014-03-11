// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest.h>

#include "net/Router.h"
#include "net/RouteRequestHandler.h"

namespace hyrise {
namespace net {

class RouterTests : public ::testing::Test {};

TEST_F(RouterTests, get_instance) { Router::getInstance(); }

TEST_F(RouterTests, test_urls_uri_existance) {
  /* This test covers static registration as performed
     by RouteRequestHandler */
  const auto& router = Router::getInstance();
  const auto& route = router.getRouters();
  ASSERT_TRUE(route.at("/urls/").get()) << "Its url is '/urls'";
}

class DummyHandler : public AbstractRequestHandler {
 public:
  explicit DummyHandler(AbstractConnection* connection) {}
  const std::string vname() { return "DummyHandler"; }
};

class DummyFactory : public AbstractRequestHandlerFactory {
 public:
  virtual AbstractRequestHandler::SharedPtr create(AbstractConnection* connection) const {
    return std::make_shared<DummyHandler>(connection);
  }
};

TEST_F(RouterTests, test_invalid_registrations) {
  Router router;
  std::unique_ptr<DummyFactory> dummy(new DummyFactory());
  ASSERT_THROW(router.addRoute("/", std::move(dummy), Router::route_t::EXACT), RouterException)
      << "'/' is not allowed.";
}

TEST_F(RouterTests, test_valid_registration_route) {
  Router router;
  std::unique_ptr<DummyFactory> dummy1(new DummyFactory());

  router.addRoute("/dummy1", std::move(dummy1), Router::route_t::EXACT);

  ASSERT_THROW(router.getHandler("/"), RouterException) << "No catch all registered, access to '/' impossible";


  std::unique_ptr<DummyFactory> dummy2(new DummyFactory());
  router.addRoute("/dummy2", std::move(dummy2), Router::route_t::CATCH_ALL);

  router.getHandler("/dummy1");
  router.getHandler("/dummy2");

  ASSERT_NE(router.getHandler("/"), nullptr) << "A handling factory should be returned";
}

TEST_F(RouterTests, test_urlmap_view_works) {
  RouteRequestHandler handler(nullptr);
  // Here we just want to make sure that construction of a response works
  handler.constructResponse();
}
}
}
