// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifdef WITH_MYSQL

#include <io/Loader.h>
#include <io/MySQLLoader.h>
#include <io/StorageManager.h>
#include "MySQLTableLoad.h"

bool MySQLTableLoad::is_registered = QueryParser::registerPlanOperation<MySQLTableLoad>();

namespace { log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan._PlanOperation")); }

MySQLTableLoad::MySQLTableLoad(): _load_limit(0) {

}

MySQLTableLoad::~MySQLTableLoad() {
  LOG4CXX_DEBUG(logger, "~MySQL Load Table");
}

void MySQLTableLoad::setDatabaseName(std::string databaseName) {
  _database_name = databaseName;
}

void MySQLTableLoad::setTableName(std::string tablename) {
  _table_name = tablename;
}


std::string MySQLTableLoad::name() {
  return "MySQLTableLoad";
}

const std::string MySQLTableLoad::vname() {
  return "MySQLTableLoad";
}


std::shared_ptr<_PlanOperation> MySQLTableLoad::parse(Json::Value &data) {
  std::shared_ptr<MySQLTableLoad> s = std::make_shared<MySQLTableLoad>();
  s->setTableName(data["table"].asString());
  s->setDatabaseName(data["database"].asString());
  if (data.isMember("limit"))
    s->setLoadLimit(data["limit"].asUInt64());
  return s;
}

void MySQLTableLoad::executePlanOperation() {
  StorageManager *sm = StorageManager::getInstance();
  std::shared_ptr<AbstractTable> t;
  if (sm->exists(_table_name)) {
    t = sm->getTable(_table_name);
    addResult(t);
  } else {
    t = Loader::load(
          Loader::params().setInput(
            MySQLInput(
              MySQLInput::params()
              .setSchema(_database_name)
              .setTable(_table_name)
              .setLimit(_load_limit)
            )
          ));
    sm->loadTable(_table_name, t);
    addResult(t);
  }
}

#endif
