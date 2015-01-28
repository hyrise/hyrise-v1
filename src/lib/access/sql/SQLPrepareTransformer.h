// Copyright (c) 2015 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLPREPARERANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLPREPARERANSFORMER_H_

#include "access/sql/typedef_helper.h"
#include "access/sql/TaskListBuilder.h"
#include "access/sql/parser/PrepareStatement.h"
#include "access/sql/parser/ExecuteStatement.h"

namespace hyrise {
namespace access {
namespace sql {

class SQLStatementTransformer;

class SQLPrepareTransformer {
 public:
  SQLPrepareTransformer(SQLStatementTransformer& server);
  virtual ~SQLPrepareTransformer();

  TransformationResult transformPrepareStatement(hsql::PrepareStatement* stmt);
  TransformationResult transformExecuteStatement(hsql::ExecuteStatement* stmt);

 private:
  SQLStatementTransformer& _server;
  TaskListBuilder& _builder;
};

} // namespace sql
} // namespace access
} // namespace hyrise
#endif