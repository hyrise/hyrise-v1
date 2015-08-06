// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "TpccStoredProcedure.h"

namespace hyrise {
namespace access {

class TpccOrderStatusProcedure : public TpccStoredProcedure {
 public:
  TpccOrderStatusProcedure(net::AbstractConnection* connection);

  virtual Json::Value execute();
  virtual void setData(const Json::Value& data);

  static std::string name();
  virtual const std::string vname();

 private:
  // queries
  storage::c_atable_ptr_t getCustomerByCId();
  storage::c_atable_ptr_t getCustomersByLastName();
  storage::c_atable_ptr_t getLastOrder();
  storage::c_atable_ptr_t getOrderLines();

  int _w_id;
  int _d_id;
  bool _customerById;
  int _c_id;
  int _o_id;
  size_t _chosenOne;
  std::string _c_last;
  std::string _date;
};
}
}  // namespace hyrise::access
