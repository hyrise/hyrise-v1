// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SpawnedTask.h"

#include <thread>
#include <random>

#include "access/BasicParser.h"
#include "access/QueryParser.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SpawnedTask>("SpawnedTask");
}

SpawnedTask::~SpawnedTask() {
}

// Executing this on a store with delta results in undefined behavior
// Execution with horizontal tables results in undefined behavior
void SpawnedTask::executePlanOperation() {
  static std::default_random_engine e((time_t) time(0));
  time_t time = e() % 25;
  std::this_thread::sleep_for(std::chrono::milliseconds(time));

}

std::shared_ptr<_PlanOperation> SpawnedTask::parse(Json::Value &data) {
  return std::make_shared<SpawnedTask>();
}

const std::string SpawnedTask::vname() {
  return "SpawnedTask";
}

}
}
