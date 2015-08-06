// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/PersistTable.h>

#include <io/StorageManager.h>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PersistTable>("PersistTable");
}

void PersistTable::executePlanOperation() {
  auto* sm = io::StorageManager::getInstance();

  if (_tableName.empty()) {
    throw std::runtime_error("PersistTable requires a table name");
  }
  if (sm->exists(_tableName)) {
    sm->persistTable(_tableName, _path);
  } else {
    throw std::runtime_error("PersistTable: Table does not exist.");
  }
}

std::shared_ptr<PlanOperation> PersistTable::parse(const Json::Value& data) {
  auto p = BasicParser<PersistTable>::parse(data);

  if (!data.isMember("tablename")) {
    throw std::runtime_error("PersistTable needs parameter tablename.");
  }
  p->_tableName = data["tablename"].asString();

  if (!data.isMember("path")) {
    p->_path = "";
  } else {
    p->_path = data["path"].asString();
  }

  return p;
}

const std::string PersistTable::vname() { return "PersistTable"; }

void PersistTable::setTableName(const std::string& name) { _tableName = name; }

void PersistTable::setPath(const std::string& path) { _path = path; }
}
}
