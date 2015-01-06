// Copyright (c) 2015 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/sql/SQLPrepareTransformer.h"
#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/PreparedStatementManager.h"
#include "access/sql/SQLQueryTask.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {


SQLPrepareTransformer::SQLPrepareTransformer(SQLStatementTransformer& server) :
  _server(server),
  _builder(server.getTaskListBuilder()) {}

SQLPrepareTransformer::~SQLPrepareTransformer() {}


TransformationResult SQLPrepareTransformer::transformPrepareStatement(PrepareStatement* stmt) {
  PreparedStatementManager& manager = PreparedStatementManager::getInstance();

  std::string name = stmt->name;
  if (manager.hasStatement(name)) {
    _server.throwError("Prepared statement of this name already exists!", name);

  } else {
    manager.addStatement(name, stmt);

  }

  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  _builder.addNoOp(meta);
  return meta;
}


TransformationResult SQLPrepareTransformer::transformExecuteStatement(ExecuteStatement* exec) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  PreparedStatementManager& manager = PreparedStatementManager::getInstance();

  std::string name = exec->name;
  if (!manager.hasStatement(name)) {
    _server.throwError("Can't execute prepared statement, because it doesn't exist!", name);

    _builder.addNoOp(meta);

  } else {
    PrepareStatement* prep = manager.getStatement(name);

    size_t num_arguments = (exec->parameters == NULL) ? 0 : exec->parameters->size();

    // Check number of parameters
    if (prep->placeholders.size() != num_arguments) {
      _server.throwError("Expected a different number of parameters!", std::to_string(prep->placeholders.size()));
    }

    // Override the placeholder expression within the prepared statement
    for (uint i = 0; i < num_arguments; ++i) {
      (*prep->placeholders[i]) = (*exec->parameters->at(i));
    }

    // Create the QueryTasks for each statement within our prepared statement
    std::shared_ptr<SQLQueryTask> last_task = nullptr;
    for (SQLStatement* stmt : prep->query->statements) {
      auto sql_task = std::make_shared<SQLQueryTask>(stmt);
      sql_task->setId(10);

      // Chain tasks
      if (last_task != nullptr) {
        sql_task->addDependency(last_task);
        sql_task->setPrevTask(last_task);
        last_task->setNextTask(sql_task);
      }
      
      last_task = sql_task;
      _builder.addPlanOp(sql_task, "SQLQueryTask", meta);
    }
  }

  return meta;
}



} // namespace sql
} // namespace access
} // namespace hyrise