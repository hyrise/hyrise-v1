// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOREDPROCEDURE_H_
#define SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOREDPROCEDURE_H_

#include "json.h"

#include <helper/types.h>
#include <io/TXContext.h>
#include <access/expressions/pred_SimpleExpression.h>
#include <net/Router.h>
#include <net/AbstractConnection.h>

namespace hyrise { namespace access {

class TpccStoredProcedure : public net::AbstractRequestHandler {
 public:
  TpccStoredProcedure(net::AbstractConnection* connection);
  virtual ~TpccStoredProcedure(){}
  
  void operator()();
  virtual Json::Value execute() = 0;
  virtual void setData(Json::Value& data) = 0;

 protected:
  //connection helper
  Json::Value data();

 protected:
  //json helper
  static Json::Value assureMemberExists(Json::Value data, const std::string name);

  //planop helper
  storage::c_atable_ptr_t getTpccTable(std::string name);
  storage::c_atable_ptr_t selectAndValidate(storage::c_atable_ptr_t table, SimpleExpression *expr);
  void insert(storage::atable_ptr_t table, storage::atable_ptr_t rows);
  void deleteRows(storage::c_atable_ptr_t rows);
  void update(storage::c_atable_ptr_t rows, Json::Value data);
  storage::c_atable_ptr_t sort(storage::c_atable_ptr_t table, field_name_t field, bool ascending);

  typedef std::vector<SimpleExpression*> expr_list_t;
  static SimpleExpression* connectAnd(expr_list_t expressions);
  
  //transaction
  void commit();
  void rollback();

  static std::string getDate();

  net::AbstractConnection* _connection;
  tx::TXContext _tx;

 private:
  void startTransaction();
  bool _finished = false;
};

} } // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOREDPROCEDURE_H_

