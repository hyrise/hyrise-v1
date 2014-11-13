// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_ACCESS_SQL_SQLQUERYPARSER_H_
#define SRC_LIB_ACCESS_SQL_SQLQUERYPARSER_H_

#include "access/system/QueryParser.h"
#include "access/sql/parser/SQLParser.h"
#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {
namespace sql {

typedef std::shared_ptr<taskscheduler::Task> task_t;
typedef std::vector<task_t> task_list_t;

class SQLQueryParser {
 public:
  SQLQueryParser();

  /**
   * Transforms the SQL string into a list of Hyrise Tasks.
   * @param[in]  query      SQL query string
   * @param[out] result  	Last task, the result of which will be the result of the query
   * @return  List of tasks that were extracted from the query
   */
  task_list_t transformSQLQuery(const std::string& query, task_t* result);


 protected:
  /**
   * Builds the task list out of the given sql query.
   * @param[in]  query      SQL query string
   * @param[out] tasks  	List of tasks that were extracted from the query
   */
  void buildTaskList(const std::string& query, task_list_t& tasks);
};

} // namespace sql
} // namespace access
} // namespace hyrise


#endif