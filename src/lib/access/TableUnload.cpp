// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/TableUnload.h>

#include <storage/storage_types.h>
#include <io/StorageManager.h>
#include <json.h>

#include "QueryParser.h"
bool TableUnload::is_registered = QueryParser::registerPlanOperation<TableUnload>();

std::shared_ptr<_PlanOperation> TableUnload::parse(Json::Value &data) {
  std::shared_ptr<TableUnload> s = std::make_shared<TableUnload>();
  s->setTableName(data["table"].asString());
  return s;
}

void TableUnload::executePlanOperation() {

  StorageManager::getInstance()->removeTable(_table_name);
}
