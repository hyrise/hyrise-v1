// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/sql/SQLQueryParser.h"
#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/predicate_helper.h"

#include "access/storage/TableLoad.h"
#include "access/storage/GetTable.h"
#include "access/SimpleTableScan.h"
#include "access/ProjectionScan.h"
#include "access/NoOp.h"
#include "access/expressions/pred_buildExpression.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

namespace {
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access")); 
}


SQLQueryParser::SQLQueryParser() {
  // Initialize
  // empty
}



task_list_t SQLQueryParser::transformSQLQuery(const std::string& query, task_t* result) {
  bool logTime = false;
  // Measure time it takes to parse the query into a list of tasks
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();


  // Build the task list
  task_list_t task_list = buildTaskList(query);

  // Result task is the last item within the task list.
  if (task_list.size() > 0) *result = task_list.back();


  // Print execution time
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  double ms = elapsed_seconds.count() * 1000.0;
  if (logTime) std::cout << "Building the task list took: " << ms << "ms\n";

  return task_list;
}




task_list_t SQLQueryParser::buildTaskList(const std::string& query) {
  // Parse the sql
  StatementList* stmt_list = SQLParser::parseSQLString(query.c_str());

  // Check if the parsing completed successfully
  if (!stmt_list->isValid) {
    throw std::runtime_error("SQL Parsing failed! " + std::string(stmt_list->parser_msg));
  }

  // Build the task list
  // TODO: Confirm that tasks are executed sequentially
  task_list_t all_tasks;
  int i = 1;
  for (Statement* stmt : stmt_list->vector()) {
    SQLStatementTransformer transformer = SQLStatementTransformer(std::to_string(i++) + ".");
    transformer.transformStatement(stmt);
    task_list_t tasks = transformer.getTaskList();
    all_tasks.insert(all_tasks.end(), tasks.begin(), tasks.end());
  }
  return all_tasks;
}


} // namespace sql
} // namespace access
} // namespace hyrise