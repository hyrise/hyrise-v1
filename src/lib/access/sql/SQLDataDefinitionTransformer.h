// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLDATADEFINITIONTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLDATADEFINITIONTRANSFORMER_H_

#include "access/sql/typedef_helper.h"
#include "access/sql/TaskListBuilder.h"
#include "access/sql/parser/CreateStatement.h"
#include "access/sql/parser/DropStatement.h"


namespace hyrise {
namespace access {
namespace sql {

class SQLStatementTransformer;


/**
 * @class SQLDataDefinitionTransformer
 * This transformer handles CREATE, DROP, ALTER, RENAME statements
 */
class SQLDataDefinitionTransformer {
 public:
  SQLDataDefinitionTransformer(SQLStatementTransformer& server);
  ~SQLDataDefinitionTransformer();

  TransformationResult transformCreateStatement(hsql::CreateStatement* stmt);
  TransformationResult transformDropStatement(hsql::DropStatement* stmt);

 private:
  SQLStatementTransformer& _server;
  TaskListBuilder& _builder;
};



} // namespace sql
} // namespace access
} // namespace hyrise
#endif