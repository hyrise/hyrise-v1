// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/RecoverTable.h>

#include <io/StorageManager.h>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<RecoverTable>("RecoverTable");
}

void RecoverTable::executePlanOperation() {
  if (_tableName.empty()) {
    throw std::runtime_error("RecoverTable requires a table name");
  }
  auto* sm = io::StorageManager::getInstance();
  sm->recoverTable(_tableName, _path, _threadCount);
}

std::shared_ptr<PlanOperation> RecoverTable::parse(const Json::Value& data) {
  auto p = BasicParser<RecoverTable>::parse(data);

  if (!data.isMember("tablename")) {
    throw std::runtime_error("RecoverTable needs parameter tablename.");
  }
  p->_tableName = data["tablename"].asString();

  if (!data.isMember("path")) {
    p->_path = "";
  } else {
    p->_path = data["path"].asString();
  }

  if (!data.isMember("threads")) {
    p->_threadCount = 1;
  } else {
    p->_threadCount = data["threads"].asInt();
  }

  return p;
}

const std::string RecoverTable::vname() { return "RecoverTable"; }

void RecoverTable::setTableName(const std::string& name) { _tableName = name; }

void RecoverTable::setPath(const std::string& path) { _path = path; }

void RecoverTable::setNumberThreads(const size_t thread_count) { _threadCount = thread_count; }
}
}
