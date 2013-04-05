// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/TableUnload.h"

#include "access/QueryParser.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<TableUnload>("TableUnload");
}

TableUnload::~TableUnload() {
}

void TableUnload::executePlanOperation() {
  StorageManager::getInstance()->removeTable(_table_name);
}

std::shared_ptr<_PlanOperation> TableUnload::parse(Json::Value &data) {
  std::shared_ptr<TableUnload> s = std::make_shared<TableUnload>();
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
