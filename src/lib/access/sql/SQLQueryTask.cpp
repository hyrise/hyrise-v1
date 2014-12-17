// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SQLQueryTask.h"

#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/parser/sqllib.h"

#include "taskscheduler/AbstractTaskScheduler.h"
#include "taskscheduler/SharedScheduler.h"
#include "io/TransactionManager.h"
#include "access/tx/Commit.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {


SQLQueryTask::SQLQueryTask(SQLStatement* stmt) :
  _stmt(stmt),
  _id(0),
  _has_commit(false),
  _next_task(nullptr),
  _prev_task(nullptr),
  _response_task(nullptr) {

}

SQLQueryTask::~SQLQueryTask() {
  delete _stmt;
}

void SQLQueryTask::executePlanOperation() {
  // Transform the statement
  SQLStatementTransformer transformer = SQLStatementTransformer(std::to_string(_id) + ".");
  transformer.transformStatement(_stmt);
  _has_commit = transformer.hasCommit();
  task_list_t tasks = transformer.getTaskList();


  // TXContext
  // Each SQL statement will be handled as a single transaction
  // If we are the first task in the chain, simply get the context of this SQLQueryTask
  // If our previouse task had no commit, we can carry over its transaction context
  // If it had a commit we begin a new transaction
  tx::TXContext ctx;
  if (_prev_task == nullptr) {
    ctx = getTXContext();
  } else if (!_prev_task->hasCommit()) {
    ctx = _prev_task->getTXContext();
  } else {
    ctx = tx::TransactionManager::beginTransaction();
  }

  // Apply all parameters from query task
  for (task_t task : tasks) {
    if (auto plan_op = std::dynamic_pointer_cast<PlanOperation>(task)) {
      plan_op->setPriority(getPriority());
      plan_op->setSessionId(getSessionId());
      plan_op->setPlanId(getPlanId());
      plan_op->setTXContext(ctx);
      plan_op->setId(getId());

      // register at response task for output
      if (_response_task != nullptr)
        _response_task->registerPlanOperation(plan_op);
    }
  }

  // Set dependency of following QueryTask/ResponseTask
  if (_next_task != nullptr) {
    _next_task->addDependency(tasks.back());
  }

  // Push tasks into scheduler
  std::shared_ptr<hyrise::taskscheduler::AbstractTaskScheduler> scheduler;
  scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();
  scheduler->scheduleQuery(tasks);
}



} // namespace sql
} // namespace access
} // namespace hyrise