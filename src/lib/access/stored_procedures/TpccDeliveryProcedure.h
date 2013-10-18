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
  //TODO
};

}} // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCDELIVERY_H_

