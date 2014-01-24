// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingRun.h"

#include <iostream>

#include <storage/storage_types.h>
#include <storage/AgingStore.h>
#include <io/StorageManager.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<AgingRun>("AgingRun");
} // namespace


AgingRun::~AgingRun() {}

void AgingRun::executePlanOperation() {
  auto& sm = *io::StorageManager::getInstance();
  const auto table = sm.getTable(_tableName);
  if (std::dynamic_pointer_cast<storage::AgingStore>(table) == nullptr) {
    std::cout << "table currently is no AgingStore :(" << std::endl;
  }
  // implement me
}

std::shared_ptr<PlanOperation> AgingRun::parse(const Json::Value &data) {
  std::shared_ptr<AgingRun> ar = std::make_shared<AgingRun>();

  if (!data.isMember("table"))
    throw std::runtime_error("A table must be specified for the AgingRun");
  ar->_tableName = data["table"].asString();

  throw std::runtime_error("implement me!");

  return ar;
}

} } // namespace hyrise::access

