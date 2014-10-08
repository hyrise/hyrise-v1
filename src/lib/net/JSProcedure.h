#pragma once

#include <string>

#include "net/Router.h"

#include "net/AsyncConnection.h"

namespace hyrise {
namespace net {

class AbstractConnection;

class JSProcedure : public AbstractRequestHandler {
  static bool registered;
  AbstractConnection* _connection;

 public:
  JSProcedure(AbstractConnection* c) : _connection(c) {}
  void operator()();
  static std::string name();
  const std::string vname();

 private:
  std::string readFile(std::string fileName);
  void showError(AsyncConnection* ac, const std::string errorMessage);
};
}
}
