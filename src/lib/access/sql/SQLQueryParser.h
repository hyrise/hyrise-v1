// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLQUERYPARSER_H_
#define SRC_LIB_ACCESS_SQL_SQLQUERYPARSER_H_

#include <access/sql/typedef_helper.h>
#include <access/system/ResponseTask.h>

namespace hyrise {
namespace access {
namespace sql {


/**
 * This object parses a given SQL query string and creates SQLQueryTask objects.
 * One task for each statement will be created.
 */
class SQLQueryParser {
 public:
  SQLQueryParser(const std::string& query, std::shared_ptr<ResponseTask> response_task);

  task_list_t buildSQLQueryTasks();

 protected:
  std::string _query;
  std::shared_ptr<ResponseTask> _response_task;

};


} // namespace sql
} // namespace access
} // namespace hyrise
#endif