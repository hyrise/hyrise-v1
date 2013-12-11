// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_NET_STATICREQUESTHANDLER_H_
#define SRC_LIB_NET_STATICREQUESTHANDLER_H_

#include <string>
#include <memory>

#include "net/Router.h"

namespace hyrise {
namespace net {

class AbstractConnection;

class StaticRequestHandler : public net::AbstractRequestHandler {
private:
  static bool _registered;
  AbstractConnection *_connection;
  std::string _rootPath;

public:
  explicit StaticRequestHandler(AbstractConnection *connection);
  virtual void operator()();
  static std::string name() { return "StaticRequestHandler"; }
  const std::string vname() { return "StaticRequestHandler"; }
};

}
}

#endif  // SRC_LIB_NET_STATICREQUESTHANDLER_H_
