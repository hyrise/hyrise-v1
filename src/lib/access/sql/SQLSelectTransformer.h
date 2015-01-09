// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLSELECTTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLSELECTTRANSFORMER_H_

#include "access/sql/typedef_helper.h"
#include "access/sql/TaskListBuilder.h"
#include "access/sql/parser/SelectStatement.h"

namespace hyrise {
namespace access {
namespace sql {

class SQLStatementTransformer;

class SQLSelectTransformer {
 public:
  SQLSelectTransformer(SQLStatementTransformer& server);
  ~SQLSelectTransformer();

  TransformationResult transformSelectStatement(hsql::SelectStatement* stmt);

 private:
  TransformationResult transformGroupByClause(hsql::SelectStatement* stmt, const TransformationResult& input);

  void transformSelectionList(hsql::SelectStatement* stmt, TransformationResult& meta);
  void transformDistinctSelection(hsql::SelectStatement* stmt, TransformationResult& meta);

  TableInfo addFieldsFromExprListToScan(plan_op_t scan, hsql::List<hsql::Expr*>* list, const TableInfo& input);

  SQLStatementTransformer& _server;
  TaskListBuilder& _builder;
};



} // namespace sql
} // namespace access
} // namespace hyrise
#endif