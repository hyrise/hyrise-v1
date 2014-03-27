// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "json.h"

#include <helper/types.h>
#include <io/TXContext.h>
#include <access/expressions/pred_SimpleExpression.h>
#include "access/system/ResponseTask.h"
#include <net/Router.h>
#include <net/AbstractConnection.h>
#include <storage/Store.h>
#include <map>

namespace hyrise {
namespace access {

class HyriseStoredProcedure : public net::AbstractRequestHandler {
 public:
  HyriseStoredProcedure(net::AbstractConnection* connection);
  virtual ~HyriseStoredProcedure() {}

  static std::string name();
  virtual const std::string vname();

  void operator()();
  net::AbstractConnection* _connection;
};
}
}  // namespace hyrise::access
