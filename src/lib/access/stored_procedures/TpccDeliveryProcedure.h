// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCDELIVERY_H_
#define SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCDELIVERY_H_

#include "TpccStoredProcedure.h"

namespace hyrise { namespace access {

class TpccDeliveryProcedure : public TpccStoredProcedure {
 public:
  TpccDeliveryProcedure(net::AbstractConnection* connection);

  virtual Json::Value execute();
  virtual void setData(Json::Value& data);

  static std::string name();
  virtual const std::string vname();

 private:
  //queries
  void deleteNewOrder();
  storage::c_atable_ptr_t getCId();
  storage::c_atable_ptr_t getNewOrder();
  storage::c_atable_ptr_t sumOLAmount();
  void updateCustomer();
  void updateOrderLine();
  void updateOrders();

  int _w_id;
  int _d_id;
  int _o_carrier_id;
  int _o_id;
  int _c_id;
  float _total;
  std::string _date;
};

}} // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCDELIVERY_H_

