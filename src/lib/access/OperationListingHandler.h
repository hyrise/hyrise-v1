#ifndef SRC_LIB_ACCESS_OPERATIONLISTINGHANDLER_H
#define SRC_LIB_ACCESS_OPERATIONLISTINGHANDLER_H

#include "net/Router.h"

namespace hyrise {
namespace net {
class AbstractConnection;
}
namespace access {

class OperationListingHandler : public net::AbstractRequestHandler {
  static bool registered;
  net::AbstractConnection* _connection_data;

 public:
  explicit OperationListingHandler(net::AbstractConnection* data);
  std::string constructResponse();
  void operator()();
  static std::string name();
  const std::string vname();
};
}
}


#endif
