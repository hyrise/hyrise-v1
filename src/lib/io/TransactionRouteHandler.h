// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <string>

#include "net/Router.h"

namespace hyrise { namespace tx {

class RequestHandler : public net::AbstractRequestHandler {
 public:
  explicit RequestHandler(net::AbstractConnection* c)
      : _connection(c) {}
  void operator()();
  virtual std::string processRequest() = 0;
 protected:
  net::AbstractConnection *_connection;
};

class TransactionStatusHandler : public RequestHandler {
 public:
  explicit TransactionStatusHandler(net::AbstractConnection *data) : RequestHandler(data) {}
  std::string processRequest() override;
  const std::string vname() { return "TransactionStatus"; }
  static std::string name() { return "TransactionStatusHandler";}
};

}}

