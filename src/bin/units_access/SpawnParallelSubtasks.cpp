// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SpawnParallelSubtasks.h"

#include "access/system/QueryParser.h"
#include "access/system/ResponseTask.h"
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SpawnParallelSubtasks>("SpawnParallelSubtasks");
}

// Executing this on a store with delta results in undefined behavior
// Execution with horizontal tables results in undefined behavior
void SpawnParallelSubtasks::executePlanOperation() {
  std::vector<std::shared_ptr<PlanOperation>> children;
  std::vector<std::shared_ptr<Task>> successors;
  auto scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();
  
  {
    std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
    for (const auto& weakDoneObserver : _doneObservers) {
      if (auto doneObserver = weakDoneObserver.lock()) {
        if (const auto task = std::dynamic_pointer_cast<Task>(doneObserver)) {
          successors.push_back(std::move(task));
        }
      }      
    }  
  }

  for (size_t i = 0; i < m_numberOfSpawns; ++i) {
    children.push_back(QueryParser::instance().parse("SpawnedTask", Json::Value())); 
    getResponseTask()->registerPlanOperation(children[i]);

    for (auto successor : successors)
      successor->addDependency(children[i]);
  }

  for (auto child : children) {
    scheduler->schedule(child);
  }

}

void SpawnParallelSubtasks::setNumberOfSpawns(const size_t number) {
  m_numberOfSpawns = number;
}

std::shared_ptr<PlanOperation> SpawnParallelSubtasks::parse(const Json::Value &data) {
  auto planOp =  std::make_shared<SpawnParallelSubtasks>();
  planOp->setNumberOfSpawns(data["amount"].asInt());
  return planOp;
}

const std::string SpawnParallelSubtasks::vname() {
  return "SpawnParallelSubtasks";
}

}
}

