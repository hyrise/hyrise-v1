// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "UnloadAll.h"

#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<UnloadAll>("UnloadAll");
}

UnloadAll::~UnloadAll() {
}

void UnloadAll::executePlanOperation() {
  io::StorageManager::getInstance()->removeAll();
}

std::shared_ptr<PlanOperation> UnloadAll::parse(const Json::Value &data) {
  std::shared_ptr<UnloadAll> s = std::make_shared<UnloadAll>();
  return s;
}

const std::string UnloadAll::vname() {
  return "UnloadAll";
}

}
}
