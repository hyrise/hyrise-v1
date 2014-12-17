// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <access/sql/SQLQueryParser.h>
#include <access/sql/parser/SQLParser.h>
#include <access/sql/SQLQueryTask.h>
#include <access/sql/SQLStatementTransformer.h>
#include <access/sql/predicate_helper.h>

#include <access/system/PlanOperation.h>
#include <access/storage/TableLoad.h>
#include <access/storage/GetTable.h>
#include <access/SimpleTableScan.h>
#include <access/ProjectionScan.h>
#include <access/NoOp.h>
#include <access/expressions/pred_buildExpression.h>

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

namespace {
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access")); 
}


SQLQueryParser::SQLQueryParser(const std::string& query, std::shared_ptr<ResponseTask> response_task) :
  _query(query),
  _response_task(response_task) {

}



task_list_t SQLQueryParser::buildSQLQueryTasks() {
  SQLStatementList* stmt_list = SQLParser::parseSQLString(_query.c_str());

  if (!stmt_list->isValid) {
    throw std::runtime_error("SQL Parsing failed! " + std::string(stmt_list->parser_msg));
  }

  task_list_t task_list;
  int i = 1;
  std::shared_ptr<SQLQueryTask> last_task = nullptr;

  for (SQLStatement* stmt : stmt_list->vector()) {
    auto sql_task = std::make_shared<SQLQueryTask>(stmt);
    sql_task->setId(i);
    sql_task->setResponseTask(_response_task);
    sql_task->setPlanOperationName("SQLQueryTask");
    sql_task->setOperatorId(std::to_string(i) + " SQLQueryTask");

    // Chain tasks
    if (last_task != nullptr) {
      sql_task->addDependency(last_task);
      sql_task->setPrevTask(last_task);
      last_task->setNextTask(sql_task);
    }

    task_list.push_back(sql_task);
    last_task = sql_task;
    i++;
  }

  // Last task has successor response task and will commit
  last_task->setNextTask(_response_task);
  _response_task->setResultTaskIndex(1);

  // Dependency of response task to the last QueryTask
  // will be set in RequestParseTask

  return task_list;
}


} // namespace sql
} // namespace access
} // namespace hyrise