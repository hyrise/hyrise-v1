// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/QueryParser.h"

#include "io/StorageManager.h"
#include "access/PlanOperation.h"


QueryParser::QueryParser() {
}

QueryParser::~QueryParser() {
  for (const auto& t: _factory)
      delete t.second;
}

std::vector<std::shared_ptr<Task> > QueryParser::deserialize(
    Json::Value query,
    std::shared_ptr<Task> *result) const {
  std::vector<std::shared_ptr<Task> > tasks;
  task_map_t task_map;

  buildTasks(query, tasks, task_map);
  setDependencies(query, task_map);
  *result = getResultTask(task_map);

  return tasks;
}

void QueryParser::buildTasks(
    const Json::Value &query,
    std::vector<std::shared_ptr<Task> > &tasks,
    task_map_t &task_map) const {
  Json::Value::Members members = query["operators"].getMemberNames();
  for (unsigned i = 0; i < members.size(); ++i) {

    Json::Value planOperationSpec = query["operators"][members[i]];
    std::string typeName = planOperationSpec["type"].asString();
    std::shared_ptr<_PlanOperation> planOperation = QueryParser::instance().parse(
        typeName, planOperationSpec);
    planOperation->setEvent(getPapiEventName(query));
    setInputs(planOperation, planOperationSpec);
    planOperation->setPart(planOperationSpec["part"].asUInt());
    planOperation->setCount(planOperationSpec["count"].asUInt());
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
    std::shared_ptr<_PlanOperation> planOperation,
    const Json::Value &planOperationSpec) const {
  //  TODO: input implies table input at this moment
  for (unsigned j = 0; j < planOperationSpec["input"].size(); ++j) {
    planOperation->addInput(StorageManager::getInstance()->getTable(
        planOperationSpec["input"][j].asString()));
  }
}

std::string QueryParser::getPapiEventName(const Json::Value &query) const {
  if (query.isMember("papi"))
    return query["papi"].asString();
  else
    return "PAPI_TOT_INS";
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

    std::shared_ptr<Task> src = task_map[currentEdge[0u].asString()];
    std::shared_ptr<Task> dst = task_map[currentEdge[1u].asString()];
    if (src != dst) {
      dst->addDependency(src);
    }
  }
}

std::shared_ptr<Task>  QueryParser::getResultTask(
    const task_map_t &task_map) const {
  std::map<Json::Value, std::shared_ptr<Task> >::const_iterator it;
  std::shared_ptr<Task> currentTask, resultTask = nullptr;

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

std::shared_ptr<_PlanOperation> QueryParser::parse(std::string name, Json::Value d) {
  if (_factory.count(name) == 0)
    throw std::runtime_error("Operator of type " + name + " not supported");
  auto op = _factory[name]->parse(d);
  op->setPlanOperationName(name);
  return op;
}


QueryParser &QueryParser::instance() {
  static QueryParser p;
  return p;
}
