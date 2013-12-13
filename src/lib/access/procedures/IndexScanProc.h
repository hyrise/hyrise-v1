#pragma once
// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <string>
#include "net/Router.h"

namespace hyrise { namespace access {

class IndexScanProcedure : public net::AbstractRequestHandler {

  static bool registered;
  net::AbstractConnection *_connection_data;
  
public:
      
  explicit IndexScanProcedure(net::AbstractConnection *data);

  void operator()();


  static std::string name() { return "IndexScanProc"; }
  const std::string vname() { return "IndexScanProc"; }


};


}}

