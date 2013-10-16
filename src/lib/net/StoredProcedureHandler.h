// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_NET_STOREDPROCEDUREHANDLER_H_
#define SRC_LIB_NET_STOREDPROCEDUREHANDLER_H_

#include <string>

#include "net/Router.h"

namespace hyrise {
namespace net {
class AbstractConnection;

class StoredProcedureHandler : public AbstractRequestHandler {
  static bool registered;
  AbstractConnection *_connection_data;
 public:
  explicit StoredProcedureHandler(AbstractConnection *data);
  std::string constructResponse();
  void operator()();
  static std::string name();
  const std::string vname();
};
}
}

#endif  // SRC_LIB_NET_STOREDPROCEDUREHANDLER_H_
