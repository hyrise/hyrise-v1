#pragma once

#include <string>

#include "net/Router.h"

namespace hyrise {
namespace net {

class AbstractConnection;

class ShutdownHandler : public AbstractRequestHandler {
  static bool registered;
  AbstractConnection* _connection;
 public:
  ShutdownHandler(AbstractConnection* c) : _connection(c) {}
  void operator()();
  static std::string name();
  const std::string vname();
};

}}

