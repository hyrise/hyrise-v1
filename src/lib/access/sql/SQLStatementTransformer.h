// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_

#include <access/sql/typedef_helper.h>
#include <access/sql/SQLQueryParser.h>
#include <access/sql/parser/SQLParser.h>
#include <access/tx/Commit.h>

namespace hyrise {
namespace access {
namespace sql {

/**
 * This object will transform a given SQLStatement object into
 * a list of tasks that correspond to the statement.
 * If the transformation is not possible it will throw a std::runtime_error.
 */
class SQLStatementTransformer {
 public:
  /**
   * Initializes the SQL Statement transformer with a prefix for the ids of the operator
   * @param[in]  id_prefix  Prefix that will be used for all IDs of the the operators created in this instance
   */
  SQLStatementTransformer(std::string id_prefix);

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

  /**
   * Get the list of tasks that the transformation generated
   * @return List of tasks
   */
  inline const task_list_t& getTaskList() const { return _task_list; }

  inline bool hasCommit() const { return _has_commit; }

 protected:
  /**
   * Append a plan op to the given task list and set some meta information according to the parameters
   * @param[in]  task       The operator to be added
   * @param[in]  name       The name of the operator
   */
  plan_op_t addPlanOp(plan_op_t task, std::string name);

  /**
   * Create a new PlanOperator object and append to the task list
   * @param[in]  Name that will be set on the operator
   * @param[in]  Dependency that will be attached to the operator (ignored if null)
   * @return  Created plan operator
   */
  template<typename _T>
  std::shared_ptr<_T> addNewPlanOp(std::string name, task_t dependency);

  template<typename _T>
  std::shared_ptr<_T> addNewPlanOp(std::string name, TransformationResult& meta);

  template<typename _T>
  std::shared_ptr<_T> addNewPlanOp(std::string name) { return addNewPlanOp<_T>(name, nullptr); }


  std::shared_ptr<Commit> addCommit(TransformationResult& meta);


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

  /**
   * Returns the next ID for an operator
   * @return  next ID for an operator
   */
  inline std::string nextTaskId();




  // Documentation for the protected methods can be found in the .cpp file
  TransformationResult transformCreateStatement(hsql::CreateStatement* create);
  TransformationResult transformInsertStatement(hsql::InsertStatement* insert);
  TransformationResult transformDeleteStatement(hsql::DeleteStatement* del);
  TransformationResult transformSelectStatement(hsql::SelectStatement* stmt);
  TransformationResult transformUpdateStatement(hsql::UpdateStatement* update);
  TransformationResult transformDropStatement(hsql::DropStatement* drop);

  TransformationResult transformGroupByClause(hsql::SelectStatement* stmt);
  TransformationResult transformSelectionList(hsql::SelectStatement* stmt, const TransformationResult& input);
  TransformationResult transformTableRef(hsql::TableRef* table);
  TransformationResult transformJoinTable(hsql::TableRef* table);
  TransformationResult transformScanJoin(hsql::TableRef* table);
  TransformationResult transformHashJoin(hsql::TableRef* table);


  // Members
  task_list_t _task_list;
  std::string _id_prefix;
  int _last_task_id;
  bool _has_commit;



};




} // namespace sql
} // namespace access
} // namespace hyrise
#endif