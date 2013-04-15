// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SpawnConsecutiveSubtasks.h"

#include "access/BasicParser.h"
#include "access/QueryParser.h"
#include "access/ResponseTask.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SpawnConsecutiveSubtasks>("SpawnConsecutiveSubtasks");
}

SpawnConsecutiveSubtasks::~SpawnConsecutiveSubtasks() {
}

// Executing this on a store with delta results in undefined behavior
// Execution with horizontal tables results in undefined behavior
void SpawnConsecutiveSubtasks::executePlanOperation() {
  std::vector<std::shared_ptr<_PlanOperation>> children;
  std::vector<Task*> successors;
  auto scheduler = SharedScheduler::getInstance().getScheduler();
  
  for (auto doneObserver : _doneObservers) {
    Task* const task = dynamic_cast<Task*>(doneObserver);
    successors.push_back(task);
  }

  for (size_t i = 0; i < m_numberOfSpawns; ++i) {
    children.push_back(QueryParser::instance().parse("SpawnedTask", Json::Value()));

    if (i != 0)
      children[i]->addDependency(children[i-1]);
  }
    
  for (auto successor : successors)
    successor->addDependency(children.back());

  for (auto child : children) {
    getResponseTask()->registerOperation(child.get());
    scheduler->schedule(child);
  }
}

void SpawnConsecutiveSubtasks::setNumberOfSpawns(const size_t number)
{
  m_numberOfSpawns = number;
}

std::shared_ptr<_PlanOperation> SpawnConsecutiveSubtasks::parse(Json::Value &data) {
  auto planOp = std::make_shared<SpawnConsecutiveSubtasks>();
  planOp->setNumberOfSpawns(data["amount"].asInt());
  return planOp;
}

const std::string SpawnConsecutiveSubtasks::vname() {
  return "SpawnConsecutiveSubtasks";
}

}
}
