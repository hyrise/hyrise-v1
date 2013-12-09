// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableUnload.h"

#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<TableUnload>("TableUnload");
}

TableUnload::~TableUnload() {
}

void TableUnload::executePlanOperation() {
  io::StorageManager::getInstance()->removeTable(_table_name);
}

std::shared_ptr<PlanOperation> TableUnload::parse(const Json::Value &data) {
  auto s = std::make_shared<TableUnload>();
  s->setTableName(data["table"].asString());
  return s;
}

const std::string TableUnload::vname() {
  return "TableUnload";
}

void TableUnload::setTableName(const std::string &tablename) {
  _table_name = tablename;
}

}
}
