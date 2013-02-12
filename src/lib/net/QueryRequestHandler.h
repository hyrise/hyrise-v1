// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_NET_QUERYREQUESTHANDLER_H_
#define SRC_LIB_NET_QUERYREQUESTHANDLER_H_

#include <string>
#include <memory>

#include "net/Router.h"

namespace hyrise { namespace access {
  class RequestParseTask;
  class ResponseTask;
}}

namespace hyrise {
namespace net {

class AbstractConnection;

class QueryRequestHandler : public net::AbstractRequestHandler {
 private:
  
  AbstractConnection *_connection;
  hyrise::access::RequestParseTask *_parseTask;


 public:

  explicit QueryRequestHandler(net::AbstractConnection *connection);
  
  virtual ~QueryRequestHandler();

  std::shared_ptr<hyrise::access::ResponseTask> getResponseTask() const;

  virtual void operator()();

  static std::string name();

  const std::string vname();

};

}
}

#endif  // SRC_LIB_NET_QUERYREQUESTHANDLER_H_
