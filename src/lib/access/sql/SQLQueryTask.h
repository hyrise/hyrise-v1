// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLQUERYTASK_H_
#define SRC_LIB_ACCESS_SQL_SQLQUERYTASK_H_

#include <access/sql/parser/SQLStatement.h>
#include <access/sql/typedef_helper.h>
#include <access/system/PlanOperation.h>
#include <access/system/ResponseTask.h>

namespace hyrise {
namespace access {
namespace sql {


/**
 * This task handles the execution of already parsed SQL queries.
 * The SQLQueryParser creates a list of these tasks.
 * Within a SQLQueryTask a SQLStatement object will be transformed into tasks
 * and the created tasks are pushed to Scheduler.
 */ 
class SQLQueryTask : public PlanOperation {
 public:
  SQLQueryTask(hsql::SQLStatement* stmt);
  virtual ~SQLQueryTask();

  void executePlanOperation();

  inline void setNextTask(task_t task) { _next_task = task; }
  inline void setPrevTask(std::shared_ptr<SQLQueryTask> task) { _prev_task = task; }
  inline void setPrefix(std::string prefix) { _prefix = prefix; }
  inline void setResponseTask(std::shared_ptr<ResponseTask> task) { _response_task = task; }

  inline bool hasCommit() const { return _has_commit; }

  const std::string vname() { return "SQLQueryTask"; }

 private:
  hsql::SQLStatement* _stmt;
  std::string _prefix;
  bool _has_commit;

  task_t _next_task;
  std::shared_ptr<SQLQueryTask> _prev_task;

  std::shared_ptr<ResponseTask> _response_task;
};


} // namespace sql
} // namespace access
} // namespace hyrise
#endif