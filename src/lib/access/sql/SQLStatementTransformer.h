// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_

#include <algorithm>

#include "access/sql/SQLQueryParser.h"

namespace hyrise {
namespace access {
namespace sql {

typedef struct TransformationResult TransformationResult;

class SQLStatementTransformer {
 public:

  /**
   * Initializes the SQL Statement transformer with a prefix for the ids of the operator
   * @param[in]  id_prefix  Prefix that will be used for all IDs of the the operators created in this instance
   */
  SQLStatementTransformer(std::string id_prefix);


  /**
   * Transforms the given statement into tasks and saves those in the specified task_list reference.
   * @param[in]  stmt       Statement that will be transformed
   * @param[out] task_list  Reference to total task list
   * @return  Information about the result of the transformation
   */
  TransformationResult transformStatement(hsql::SQLStatement* stmt);


  inline task_list_t getTaskList() { return _task_list; }


  /**
   * Throws a runtime execption
   * @param[in] msg   Message describing the error.
   */
  static inline void throwError(std::string msg) { throw std::runtime_error("Error when transforming SQL: " + msg); }
  static inline void throwError(std::string msg, std::string detail) { throw std::runtime_error("Error when transforming SQL: " + msg + " (" + detail + ")"); }

 protected:
  // Documentation for the protected methods can be found in the .cpp file
  TransformationResult transformCreateStatement(hsql::CreateStatement* create);
  TransformationResult transformInsertStatement(hsql::InsertStatement* insert);
  TransformationResult transformDeleteStatement(hsql::DeleteStatement* del);
  TransformationResult transformSelectStatement(hsql::SelectStatement* stmt);
  TransformationResult transformDropStatement(hsql::DropStatement* drop);

  TransformationResult transformGroupByClause(hsql::SelectStatement* stmt);
  TransformationResult transformSelectionList(hsql::SelectStatement* stmt, TransformationResult info);
  TransformationResult transformTableRef(hsql::TableRef* table, bool validate = true);
  TransformationResult transformJoinTable(hsql::TableRef* table);
  TransformationResult transformScanJoin(hsql::TableRef* table);
  TransformationResult transformHashJoin(hsql::TableRef* table);

  TransformationResult addGetTable(std::string name);
  std::shared_ptr<PlanOperation> addFilterOpFromExpr(hsql::Expr* expr);

  template<typename _T>
  std::shared_ptr<_T> addOperator(std::string id, task_t dependency);

  
  /**
   * Returns the next ID for an operator
   * @return  next ID for an operator
   */
  inline std::string nextTaskId() { return _id_prefix + std::to_string(++_last_task_id); }

  /**
   * Append a plan op to the given task list and set some meta information according to the parameters
   * @param[in]  task       The operator to be added
   * @param[in]  name       The name of the operator
   */
  inline void appendPlanOp(std::shared_ptr<PlanOperation> task, std::string name) {
    task->setPlanOperationName(name);
    task->setOperatorId(nextTaskId() + " " + name);
    _task_list.push_back(task);
  }

  // Members
  task_list_t _task_list;
  std::string _id_prefix;
  int _last_task_id;

};


struct TableInfo {
  // Information about the table
  std::vector<std::string> fields;
  std::vector<DataType> data_types;
  std::string name;

  inline void addField(std::string field) { fields.push_back(field); }
  inline void addField(std::string field, DataType type) { addField(field); data_types.push_back(type); }
  inline bool containsField(std::string field) { return std::find(fields.begin(), fields.end(), field) != fields.end(); }
  inline bool hasName(std::string name2) { return name2.compare(name) == 0; }
  inline size_t numColumns() { return fields.size(); }
};


struct TransformationResult : TableInfo {
  task_t first_task;
  task_t last_task;


  void append(TransformationResult other) {
    fields     = other.fields;
    data_types = other.data_types;
    name       = other.name;
    last_task  = other.last_task;
  }
};

// Zero initializes a Transformation Result struct without causing warnings
// TransformationResult t = ALLOC_TRANSFORMATIONRESULT();
#define ALLOC_TRANSFORMATIONRESULT() {}






} // namespace sql
} // namespace access
} // namespace hyrise
#endif