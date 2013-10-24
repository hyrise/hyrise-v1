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
  //planop helper
  static storage::c_atable_ptr_t getTpccTable(std::string name, const tx::TXContext& tx);
  static storage::c_atable_ptr_t selectAndValidate(storage::c_atable_ptr_t table, SimpleExpression *expr, const tx::TXContext& tx);
  static void insert(storage::atable_ptr_t table, storage::atable_ptr_t rows, const tx::TXContext& tx);
  static void deleteRows(storage::c_atable_ptr_t rows, const tx::TXContext& tx);
  static void update(storage::c_atable_ptr_t rows, Json::Value data, const tx::TXContext& tx);
  static storage::c_atable_ptr_t sort(storage::c_atable_ptr_t table, field_name_t field, bool ascending, const tx::TXContext& tx);

  typedef std::vector<SimpleExpression*> expr_list_t;
  static SimpleExpression* connectAnd(expr_list_t expressions);
  
  typedef std::set<std::string> field_name_set_t;
  static storage::c_atable_ptr_t project(storage::c_atable_ptr_t table, field_name_set_t fields, const tx::TXContext& tx);
  
  //transaction
  static tx::TXContext startTransaction();
  static void commit(tx::TXContext tx);
  static void rollback(tx::TXContext tx);

  static std::string getDate();

  net::AbstractConnection* _connection;
  tx::TXContext _tx;
};

} } // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOREDPROCEDURE_H_

