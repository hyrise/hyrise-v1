// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SpawnParallelSubtasks.h"

#include <iostream>

#include "access/BasicParser.h"
#include "access/QueryParser.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SpawnParallelSubtasks>("SpawnParallelSubtasks");
}

SpawnParallelSubtasks::~SpawnParallelSubtasks() {
}

// Executing this on a store with delta results in undefined behavior
// Execution with horizontal tables results in undefined behavior
void SpawnParallelSubtasks::executePlanOperation() {
  std::vector<std::shared_ptr<_PlanOperation>> children;
  std::vector<Task*> successors;
  auto scheduler = SharedScheduler::getInstance().getScheduler();
  
  for (auto doneObserver : _doneObservers) {
    Task* const task = dynamic_cast<Task*>(doneObserver);
    successors.push_back(task);
  }

  for (int i = 0; i < 50; ++i) {
    children.push_back(QueryParser::instance().parse("SpawnedTask", Json::Value()));
    
    for (auto successor : successors)
      successor->addDependency(children[i]);

    scheduler->schedule(children[i]);

    std::cout << "created subtask (" << i << ")" << std::endl;
  }
}

std::shared_ptr<_PlanOperation> SpawnParallelSubtasks::parse(Json::Value &data) {
  return std::make_shared<SpawnParallelSubtasks>();
}

const std::string SpawnParallelSubtasks::vname() {
  return "SpawnParallelSubtasks";
}

}
}
