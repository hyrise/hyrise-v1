// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "TpccStoredProcedure.h"

namespace hyrise {
namespace access {

class TpccPaymentProcedure : public TpccStoredProcedure {
 public:
  TpccPaymentProcedure(net::AbstractConnection* connection);

  virtual Json::Value execute();
  virtual void setData(const Json::Value& data);

  static std::string name();
  virtual const std::string vname();

 private:
  // queries
  storage::c_atable_ptr_t getCustomerByCId();
  storage::c_atable_ptr_t getCustomersByLastName();
  storage::c_atable_ptr_t getDistrict();
  storage::c_atable_ptr_t getWarehouse();
  void insertHistory();
  void updateDistrictBalance(const storage::atable_ptr_t& districtRow);
  void updateBCCustomer(const storage::atable_ptr_t& customerRow);
  void updateGCCustomer(const storage::atable_ptr_t& customerRow);
  void updateWarehouseBalance(const storage::atable_ptr_t& warehouseRow);

  int _w_id;
  int _d_id;
  bool _customerById;
  int _c_id;
  std::string _c_last;
  int _c_w_id;
  int _c_d_id;
  bool _bc_customer;
  std::string _date;
  size_t _chosenOne;
  std::string _h_data;
  float _w_ytd;
  float _d_ytd;
  float _h_amount;
  int _c_payment_cnt;
  float _c_balance;
  float _c_ytd_payment;
  std::string _c_data;
};
}
}  // namespace hyrise::access
