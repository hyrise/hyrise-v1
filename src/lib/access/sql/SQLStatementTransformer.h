// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_


#include "access/sql/typedef_helper.h"
#include "access/sql/TaskListBuilder.h"
#include "access/sql/SQLQueryParser.h"
#include "access/sql/parser/SQLParser.h"
#include "access/sql/SQLSelectTransformer.h"
#include "access/sql/SQLDataDefinitionTransformer.h"
#include "access/sql/SQLDataManipulationTransformer.h"

namespace hyrise {
namespace access {
namespace sql {

/**
 * This object will transform a given SQLStatement object into
 * a list of tasks that correspond to the statement.
 * If the transformation is not possible it will throw a std::runtime_error.
 */
class SQLStatementTransformer {
  friend SQLSelectTransformer;
  friend SQLDataDefinitionTransformer;
  friend SQLDataManipulationTransformer;

 public:
  /**
   * Initializes the SQL Statement transformer with a prefix for the ids of the operator
   * @param[in]  id_prefix  Prefix that will be used for all IDs of the the operators created in this instance
   */
  SQLStatementTransformer(std::string id_prefix);

  virtual ~SQLStatementTransformer();

  /**
   * Transforms the given statement into tasks and saves those in a task list.
   * This list can be retrieved by calling getTaskList().
   * @param[in]  stmt       Statement that will be transformed
   * @return Information about the result of the transformation
   */
  TransformationResult transformStatement(hsql::SQLStatement* stmt);

  /**
   * Throws a runtime execption
   * @param[in] msg   Message describing the error.
   */
  static inline void throwError(std::string msg) { throw std::runtime_error("Error when transforming SQL: " + msg); }
  static inline void throwError(std::string msg, std::string detail) { throw std::runtime_error("Error when transforming SQL: " + msg + " (" + detail + ")"); }

  inline TaskListBuilder& getTaskListBuilder() { return _builder; }
  inline bool hasCommit() { return _builder.hasCommit(); }
  inline const task_list_t& getTaskList() { return _builder.getTaskList(); }

  SQLSelectTransformer* getSelectTransformer();
  SQLDataDefinitionTransformer* getDataDefinitionTransformer();
  SQLDataManipulationTransformer* getDataManipulationTransformer();

 protected:
  /**
   * Create and add a GetTable operator for the table. Validate positions if required
   * @param[in]  Table name
   * @param[in]  Flag indicating whether a ValidatePositions operator should be added
   * @return  Information about the transformation
   */
  TransformationResult addGetTable(std::string name, bool validate);

  /**
   * Create a SimpleTableScan based on the expression.
   * Uses meta information to set dependencies and updates the meta.
   * @param[in]  Filter-like expression that will be transformed into predicates
   * @param[in/out]  Information about the input. The created operator will be automatically added
   * @return  Created PlanOperator
   */
  plan_op_t addFilterOpFromExpr(hsql::Expr* expr, TransformationResult& meta);

  TransformationResult transformTableRef(hsql::TableRef* table);
  TransformationResult transformJoinTable(hsql::TableRef* table);
  TransformationResult transformHashJoin(hsql::TableRef* table);

  // Members
  TaskListBuilder _builder;
  SQLSelectTransformer* _select_transformer;
  SQLDataDefinitionTransformer* _definition_transformer;
  SQLDataManipulationTransformer* _manipulation_transformer;
};




} // namespace sql
} // namespace access
} // namespace hyrise
#endif