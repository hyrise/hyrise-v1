// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCNEWORDER_H_
#define SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCNEWORDER_H_

#include "TpccStoredProcedure.h"

namespace hyrise { namespace access {

class TpccNewOrderProcedure : public TpccStoredProcedure {
 public:
  TpccNewOrderProcedure(net::AbstractConnection* connection);

  virtual Json::Value execute();
  virtual void setData(Json::Value& data);

  static std::string name();
  virtual const std::string vname();

 private:
  //queries
  storage::c_atable_ptr_t getOId();
  storage::c_atable_ptr_t getStockCount();

  int _w_id;
  int _d_id;
  int _threshold;
  int _next_o_id;
};

} } // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCNEWORDER_H_

