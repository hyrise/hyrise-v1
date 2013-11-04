// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_NET_ABSTRACTCONNECTION_H_
#define SRC_LIB_NET_ABSTRACTCONNECTION_H_

#include <string>

namespace hyrise {
namespace net {

class AbstractConnection {
 public:
  virtual ~AbstractConnection();
  virtual std::string getBody() const = 0;
  virtual std::string getPath() const = 0;
  virtual bool hasBody() const = 0;
  virtual void respond(const std::string &message, size_t status=200, const std::string& contentType="application/json") = 0;
};

}
}

#endif
