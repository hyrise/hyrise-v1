// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/QueryParser.h"

#include "io/StorageManager.h"
#include "helper/HwlocHelper.h"
#include "helper/vector_helpers.h"
#include "access/system/PlanOperation.h"
#include "access/system/ParallelizablePlanOperation.h"

namespace hyrise { namespace access {

QueryParser::QueryParser() {
}

QueryParser::~QueryParser() {
  for (const auto& t: _factory)
      delete t.second;
}

std::vector<std::shared_ptr<taskscheduler::Task> > QueryParser::deserialize(
    const Json::Value& query,
    std::shared_ptr<taskscheduler::Task> *result) const {
  std::vector<std::shared_ptr<taskscheduler::Task> > tasks;
  task_map_t task_map;

  buildTasks(query, tasks, task_map);
  setDependencies(query, task_map);
  *result = getResultTask(task_map);

  return tasks;
}

void QueryParser::buildTasks(
    const Json::Value &query,
    std::vector<std::shared_ptr<taskscheduler::Task> > &tasks,
    task_map_t &task_map) const {
  const Json::Value::Members& members = query["operators"].getMemberNames();
  std::string papiEventName = getPapiEventName(query);
  for (unsigned i = 0; i < members.size(); ++i) {
    const Json::Value& planOperationSpec = query["operators"][members[i]];
    std::string typeName = planOperationSpec["type"].asString();
    std::shared_ptr<PlanOperation> planOperation = QueryParser::instance().parse(
        typeName, planOperationSpec);
    planOperation->setEvent(papiEventName);
    setInputs(planOperation, planOperationSpec);
    planOperation->setDynamic(planOperationSpec["dynamic"].asBool());
    if (auto para = std::dynamic_pointer_cast<ParallelizablePlanOperation>(planOperation)) {
      para->setPart(planOperationSpec["part"].asUInt());
      para->setCount(planOperationSpec["count"].asUInt());
    } else {
      if (planOperationSpec.isMember("part") || planOperationSpec.isMember("count")) {
        throw std::runtime_error("Trying to parallelize " + typeName + ", which is not a subclass of Parallelizable");
      }
    }
    
    planOperation->setOperatorId(members[i]);
    if (planOperationSpec.isMember("core"))
      planOperation->setPreferredCore(planOperationSpec["core"].asInt());
    // check for materialization strategy
    if (planOperationSpec.isMember("positions"))
      planOperation->setProducesPositions(!planOperationSpec["positions"].asBool());
    tasks.push_back(planOperation);
    task_map[members[i]] = planOperation;
  }
}

void QueryParser::setInputs(
    std::shared_ptr<PlanOperation> planOperation,
    const Json::Value &planOperationSpec) const {
  //  TODO: input implies table input at this moment
  for (unsigned j = 0; j < planOperationSpec["input"].size(); ++j) {
    planOperation->addInput(io::StorageManager::getInstance()->getTable(
        planOperationSpec["input"][j].asString()));
  }
}

std::string QueryParser::getPapiEventName(const Json::Value &query) const {
  return query.isMember("papi") ? query["papi"].asString() : "PAPI_TOT_INS";
}

int QueryParser::getSessionId(const Json::Value &query) const {
  return query.isMember("sessionId") ? query["sessionId"].asInt() : 0;
}

void QueryParser::setDependencies(
    const Json::Value &query,
    task_map_t &task_map) const {
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    Json::Value currentEdge = query["edges"][i];
    if (task_map.count(currentEdge[0u].asString()) == 0)
      throw std::runtime_error("Edege with operator name " + currentEdge[0u].asString() + " not found");

    if (task_map.count(currentEdge[1u].asString()) == 0)
      throw std::runtime_error("Edege with operator name " + currentEdge[1u].asString() + " not found");

    auto src = task_map[currentEdge[0u].asString()];
    auto dst = task_map[currentEdge[1u].asString()];
    if (src != dst) {
      dst->addDependency(src);
    }
  }
}

std::shared_ptr<taskscheduler::Task>  QueryParser::getResultTask(
    const task_map_t &task_map) const {
  std::map<Json::Value, std::shared_ptr<taskscheduler::Task> >::const_iterator it;
  std::shared_ptr<taskscheduler::Task> currentTask, resultTask = nullptr;

  for (it = task_map.begin(); it != task_map.end(); ++it) {
    currentTask = it->second;
    // Also, exclude autojson reference table task
    if (!currentTask->hasSuccessors()
        &&  it->first.asString() != autojsonReferenceTableId) {
      resultTask = currentTask;
      break;
    }
  }
  return resultTask;
}

std::shared_ptr<PlanOperation> QueryParser::parse(std::string name, const Json::Value& d) {
  if (_factory.count(name) == 0)
    throw std::runtime_error("Operator of type " + name + " not supported");
  auto op = _factory[name]->parse(d);
  op->setPlanOperationName(name);
  return op;
}


std::vector<std::string> QueryParser::getOperationNames() const {
  std::vector<std::string> result;
  for (const auto& p: _factory) {
    result.push_back(p.first);
  }
  return result;
}

QueryParser &QueryParser::instance() {
  static QueryParser p;
  return p;
}

}}
