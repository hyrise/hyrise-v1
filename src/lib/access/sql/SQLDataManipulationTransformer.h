// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLDATAMANIPULATIONTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLDATAMANIPULATIONTRANSFORMER_H_

#include "access/sql/typedef_helper.h"
#include "access/sql/TaskListBuilder.h"
#include "access/sql/parser/sqllib.h"


namespace hyrise {
namespace access {
namespace sql {

class SQLStatementTransformer;


/**
 * @class SQLDataManipulationTransformer
 * This transformer handles INSERT, UPDATE, DELETE statements
 */
class SQLDataManipulationTransformer {
 public:
  SQLDataManipulationTransformer(SQLStatementTransformer& server);
  ~SQLDataManipulationTransformer();

  TransformationResult transformUpdateStatement(hsql::UpdateStatement* stmt);
  TransformationResult transformDeleteStatement(hsql::DeleteStatement* stmt);
  TransformationResult transformInsertStatement(hsql::InsertStatement* stmt);

 private:
  SQLStatementTransformer& _server;
  TaskListBuilder& _builder;
};



} // namespace sql
} // namespace access
} // namespace hyrise
#endif