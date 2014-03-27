// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "TpccStoredProcedure.h"

namespace hyrise {
namespace access {

class TpccStockLevelProcedure : public TpccStoredProcedure {
 public:
  TpccStockLevelProcedure(net::AbstractConnection* connection);

  virtual Json::Value execute();
  virtual void setData(const Json::Value& data);

  static std::string name();
  virtual const std::string vname();

 private:
  // queries
  storage::c_atable_ptr_t getOId();
  hyrise_int_t getStockCount();

  int _w_id;
  int _d_id;
  int _threshold;
  int _next_o_id;
};
}
}  // namespace hyrise::access
