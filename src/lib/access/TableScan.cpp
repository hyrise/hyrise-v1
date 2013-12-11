// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/TableScan.h"

#include "access/expressions/ExampleExpression.h"
#include "access/expressions/pred_SimpleExpression.h"
#include "access/expressions/ExpressionRegistration.h"
#include "storage/PointerCalculator.h"
#include "storage/TableRangeView.h"
#include "helper/types.h"
#include "helper/make_unique.h"

#include "log4cxx/logger.h"

#include "access/UnionAll.h"
#include "access/system/ResponseTask.h"

namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<TableScan>("TableScan");
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
}

TableScan::TableScan(std::unique_ptr<AbstractExpression> expr) : _expr(std::move(expr)) {}

void TableScan::setupPlanOperation() {
  const auto& table = getInputTable();
  auto tablerange = std::dynamic_pointer_cast<const storage::TableRangeView>(table);
  if(tablerange)
    _expr->walk({tablerange->getActualTable()});
  else
    _expr->walk({table});
}

void TableScan::executePlanOperation() {
  size_t start, stop;
  const auto& tablerange = std::dynamic_pointer_cast<const storage::TableRangeView>(getInputTable());
  if(tablerange){
    start = tablerange->getStart();
    stop = start + tablerange->size();
  } else {
    start = 0;
    stop = getInputTable()->size();
  }

  // When the input is 0, dont bother trying to generate results
  pos_list_t* positions = nullptr;
  if(stop - start > 0)
    positions = _expr->match(start, stop);
  else
    positions = new pos_list_t();

  std::shared_ptr<storage::PointerCalculator> result;

  if(tablerange)
    result = storage::PointerCalculator::create(tablerange->getActualTable(), positions);
  else
    result = storage::PointerCalculator::create(getInputTable(), positions);

  addResult(result);
}

std::shared_ptr<PlanOperation> TableScan::parse(const Json::Value& data) {
  return std::make_shared<TableScan>(Expressions::parse(data["expression"].asString(), data));
}

size_t TableScan::getTotalTableSize() {
  const auto& dep = std::dynamic_pointer_cast<PlanOperation>(_dependencies[0]);
  auto& inputTable = dep->getResultTable();
  return inputTable->size();
}

std::vector<taskscheduler::task_ptr_t> TableScan::applyDynamicParallelization(size_t dynamicCount){

  std::vector<taskscheduler::task_ptr_t> tasks;

  // if no parallelization is necessary, just return this task again as is
  if (dynamicCount <= 1) {
    tasks.push_back(shared_from_this());
    return tasks;
  }

  std::vector<taskscheduler::task_ptr_t> successors;
  {
    std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
    // get successors of current task
    for (auto doneObserver : _doneObservers) {
      auto const task = std::dynamic_pointer_cast<taskscheduler::Task>(doneObserver.lock());
      successors.push_back(task);
    }
    // remove done observers from current task
    _doneObservers.clear();
  }

  // set part and count for this task as first task
  setPart(0);
  setCount(dynamicCount);
  tasks.push_back(std::static_pointer_cast<taskscheduler::Task>(shared_from_this()));
  std::string opIdBase = _operatorId;
  _operatorId = opIdBase + "_0";

  // create other TableScans
  for(size_t i = 1; i < dynamicCount; i++){
    auto t = std::make_shared<TableScan>(_expr->clone());

    t->setOperatorId(opIdBase + "_" + std::to_string(i));

    // build tabletask
    t->setProducesPositions(producesPositions);
    t->setPart(i);
    t->setCount(dynamicCount);
    t->setPriority(_priority);
    t->setSessionId(_sessionId);
    t->setPlanId(_planId);
    t->setTXContext(_txContext);
    t->setId(_txContext.tid);
    t->setEvent(_papiEvent);

    // set dependencies equal to current task
    for(auto d : _dependencies)
      t->addDoneDependency(d);

    t->setPlanOperationName("TableScan");
    if (auto responseTask = getResponseTask()) {
      responseTask->registerPlanOperation(t);
    }

    tasks.push_back(t);
  }

  // create union and set dependencies
  auto unionall = std::make_shared<UnionAll>();
  unionall->setPlanOperationName("UnionAll");
  unionall->setOperatorId(opIdBase + "_union");
  unionall->setProducesPositions(producesPositions);
  unionall->setPriority(_priority);
  unionall->setSessionId(_sessionId);
  unionall->setPlanId(_planId);
  unionall->setTXContext(_txContext);
  unionall->setId(_txContext.tid);
  unionall->setEvent(_papiEvent);


  for(auto t: tasks)
    unionall->addDependency(t);

  // set union as dependency to all successors
  for (auto successor : successors)
    successor->changeDependency(std::dynamic_pointer_cast<taskscheduler::Task>(shared_from_this()), unionall);

  if (auto responseTask = getResponseTask()) {
    responseTask->registerPlanOperation(unionall);
  }

  tasks.push_back(unionall);

  return tasks;
}


}}
