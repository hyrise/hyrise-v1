// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "MySQLTableLoad.h"

#include "io/Loader.h"
#include "io/MySQLLoader.h"
#include "io/StorageManager.h"

#ifdef WITH_MYSQL

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<MySQLTableLoad>("MySQLTableLoad");
}

MySQLTableLoad::MySQLTableLoad(): _load_limit(0) {
}

MySQLTableLoad::~MySQLTableLoad() {
}

void MySQLTableLoad::executePlanOperation() {
  auto sm = io::StorageManager::getInstance();
  storage::atable_ptr_t t;
  if (sm->exists(_table_name)) {
    t = sm->getTable(_table_name);
    addResult(t);
  } else {
    t = io::Loader::load(
          io::Loader::params().setInput(
            io::MySQLInput(
              io::MySQLInput::params()
              .setSchema(_database_name)
              .setTable(_table_name)
              .setLimit(_load_limit)
            )
          ));
    sm->loadTable(_table_name, t);
    addResult(t);
  }
}

std::shared_ptr<PlanOperation> MySQLTableLoad::parse(const Json::Value &data) {
  std::shared_ptr<MySQLTableLoad> s = std::make_shared<MySQLTableLoad>();
  s->setTableName(data["table"].asString());
  s->setDatabaseName(data["database"].asString());
  if (data.isMember("limit"))
    s->setLoadLimit(data["limit"].asUInt64());
  return s;
}

const std::string MySQLTableLoad::vname() {
  return "MySQLTableLoad";
}

void MySQLTableLoad::setDatabaseName(const std::string &databaseName) {
  _database_name = databaseName;
}

void MySQLTableLoad::setTableName(const std::string &tablename) {
  _table_name = tablename;
}

void MySQLTableLoad::setLoadLimit(const uint64_t l) {
  _load_limit = l;
}

}
}

#endif
