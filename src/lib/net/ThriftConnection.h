// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_NET_THRIFTCONNECTION_H_
#define SRC_LIB_NET_THRIFTCONNECTION_H_

#include <string>
#include "AbstractConnection.h"

namespace hyrise {
namespace net {

class ThriftConnection : public AbstractConnection {

  const std::string _query;
  std::string response;

 public:
  
  ThriftConnection(std::string q) : _query(q) {}

  virtual std::string getBody() const { return _query; }
  virtual std::string getPath() const { return "";}
  virtual bool hasBody() const { return true; }
  virtual void respond(const std::string &message, size_t status=200, const std::string& contentType="application/json") { response = message; }

  std::string getResponse() { return response; }
};

}
}

#endif //SRC_LIB_NET_THRIFTCONNECTION_H_
