// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLSTATEMENTTRANSFORMER_H_


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
  TransformationResult transformStatement(hsql::Statement* stmt, task_list_t& task_list);


 protected:
  // Documentation for the protected methods can be found in the .cpp file

  TransformationResult transformCreateStatement(hsql::CreateStatement*, task_list_t&);
  TransformationResult transformSelectStatement(hsql::SelectStatement*, task_list_t&);
  TransformationResult transformGroupByClause(hsql::SelectStatement*, task_list_t&);
  TransformationResult transformSelectionList(hsql::SelectStatement*, TransformationResult info, task_list_t&);
  TransformationResult transformTableRef(hsql::TableRef*, task_list_t&);

  /**
   * Throws a runtime execption
   * @param[in] msg   Message describing the error.
   */
  inline void throwError(std::string msg) { throw std::runtime_error("Error when transforming SQL: " + msg); }
  
  /**
   * Returns the next ID for an operator
   * @return  next ID for an operator
   */
  inline std::string nextTaskId() { return id_prefix + std::to_string(++last_task_id); }

  /**
   * Append a plan op to the given task list and set some meta information according to the parameters
   * @param[in]  task       The operator to be added
   * @param[in]  name       The name of the operator
   * @param[out] task_list  Task list that will be appended to
   */
  inline void appendPlanOp(std::shared_ptr<PlanOperation> task, std::string name, task_list_t& task_list) {
    task->setPlanOperationName(name);
    task->setOperatorId(nextTaskId() + " " + name);
    task_list.push_back(task);
  }


  // Members

  std::string id_prefix;
  int last_task_id;

};



struct TransformationResult {
  task_t first_task;
  task_t last_task;

  // Information about the table
  int num_columns;
  std::vector<std::string> column_names;
  std::vector<DataType> data_types;

  inline void addField(std::string field) { column_names.push_back(field); num_columns = column_names.size(); }
  inline void addField(std::string field, DataType type) { addField(field); data_types.push_back(type); }
};

// Zero initializes a Transformation Result struct without causing warnings
#define ALLOC_TRANSFORMATIONRESULT() { NULL, NULL, 0, std::vector<std::string>(), std::vector<DataType>() }


} // namespace sql
} // namespace access
} // namespace hyrise




#endif