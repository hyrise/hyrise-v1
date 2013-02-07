// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "UnloadAll.h"

#include <storage/storage_types.h>
#include <io/StorageManager.h>
#include <json.h>

#include "QueryParser.h"
bool UnloadAll::is_registered = QueryParser::registerPlanOperation<UnloadAll>();



std::shared_ptr<_PlanOperation> UnloadAll::parse(Json::Value &data) {
  std::shared_ptr<UnloadAll> s = std::make_shared<UnloadAll>();
  return s;
}

void UnloadAll::executePlanOperation() {
  StorageManager::getInstance()->removeAll();
}
