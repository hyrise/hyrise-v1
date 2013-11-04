// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SpawnedTask.h"

#include <thread>
#include <random>

#include "access/system/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SpawnedTask>("SpawnedTask");
}

void SpawnedTask::executePlanOperation() {
  static std::default_random_engine e((time_t) time(0));
  time_t time = e() % 25;
  std::this_thread::sleep_for(std::chrono::milliseconds(time));

}

std::shared_ptr<PlanOperation> SpawnedTask::parse(const Json::Value &data) {
  return std::make_shared<SpawnedTask>();
}

const std::string SpawnedTask::vname() {
  return "SpawnedTask";
}

}
}

