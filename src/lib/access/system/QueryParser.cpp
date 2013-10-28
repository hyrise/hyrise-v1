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

std::vector<std::shared_ptr<Task> > QueryParser::deserialize(
    const rapidjson::Value& query,
    std::shared_ptr<Task> *result) const {
  std::vector<std::shared_ptr<Task> > tasks;
  task_map_t task_map;

  buildTasks(query, tasks, task_map);
  setDependencies(query, task_map);
  *result = getResultTask(task_map);

  return tasks;
}

void QueryParser::buildTasks(
    const rapidjson::Value &query,
    std::vector<std::shared_ptr<Task> > &tasks,
    task_map_t &task_map) const {


  std::string papiEventName = getPapiEventName(query);
  const auto& members = query["operators"].getMemberNames();

  for(auto it = query["operators"].MemberBegin(); it != query["operators"].MemberEnd(); ++it) {

    const rapidjson::Value& planOperationSpec = it->value;
    std::string typeName = planOperationSpec["type"].GetString();

    std::shared_ptr<PlanOperation> planOperation = QueryParser::instance().parse(
        typeName, planOperationSpec);

    planOperation->setEvent(papiEventName);
    setInputs(planOperation, planOperationSpec);

    if (auto para = std::dynamic_pointer_cast<ParallelizablePlanOperation>(planOperation)) {
      para->setPart(planOperationSpec["part"].GetUint());
      para->setCount(planOperationSpec["count"].GetUint());
    } else {
      if (planOperationSpec.HasMember("part") || planOperationSpec.HasMember("count")) {
        throw std::runtime_error("Trying to parallelize " + typeName + ", which is not a subclass of Parallelizable");
      }
    }
    
    planOperation->setOperatorId(it->name.GetString());
    if (planOperationSpec.HasMember("core"))
      planOperation->setPreferredCore(planOperationSpec["core"].GetInt());
    // check for materialization strategy
    if (planOperationSpec.HasMember("positions"))
      planOperation->setProducesPositions(!planOperationSpec["positions"].GetBool());
    
    tasks.push_back(planOperation);
    std::string key = it->name.GetString();
    task_map[key] = planOperation;

  }

  // for (unsigned i = 0; i < members.size(); ++i) {
  //   rapidjson::Value planOperationSpec = query["operators"][members[i]];
  //   std::string typeName = planOperationSpec["type"].asString();
  //   std::shared_ptr<PlanOperation> planOperation = QueryParser::instance().parse(
  //       typeName, planOperationSpec);
  //   planOperation->setEvent(papiEventName);
  //   setInputs(planOperation, planOperationSpec);
  //   if (auto para = std::dynamic_pointer_cast<ParallelizablePlanOperation>(planOperation)) {
  //     para->setPart(planOperationSpec["part"].asUInt());
  //     para->setCount(planOperationSpec["count"].asUInt());
  //   } else {
  //     if (planOperationSpec.isMember("part") || planOperationSpec.isMember("count")) {
  //       throw std::runtime_error("Trying to parallelize " + typeName + ", which is not a subclass of Parallelizable");
  //     }
  //   }
    
  //   planOperation->setOperatorId(members[i]);
  //   if (planOperationSpec.isMember("core"))
  //     planOperation->setPreferredCore(planOperationSpec["core"].asInt());
  //   // check for materialization strategy
  //   if (planOperationSpec.isMember("positions"))
  //     planOperation->setProducesPositions(!planOperationSpec["positions"].asBool());
  //   tasks.push_back(planOperation);
  //   task_map[members[i]] = planOperation;
  // }
}

void QueryParser::setInputs(
    std::shared_ptr<PlanOperation> planOperation,
    const rapidjson::Value &planOperationSpec) const {
  //  TODO: input implies table input at this moment
  for (unsigned j = 0; j < planOperationSpec["input"].size(); ++j) {
    planOperation->addInput(StorageManager::getInstance()->getTable(
        planOperationSpec["input"][j].asString()));
  }
}

std::string QueryParser::getPapiEventName(const rapidjson::Value &query) const {
  if (query.isMember("papi"))
    return query["papi"].asString();
  else
    return "PAPI_TOT_INS";
}

int QueryParser::getSessionId(const rapidjson::Value &query) const {
  if (query.isMember("sessionId"))
    return query["sessionId"].asInt();
  else
    return 0;
}

void QueryParser::setDependencies(
    const rapidjson::Value &query,
    task_map_t &task_map) const {
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    const rapidjson::Value& currentEdge = query["edges"][i];
    if (task_map.count(currentEdge[0u].asString()) == 0)
      throw std::runtime_error("Edege with operator name " + currentEdge[0u].asString() + " not found");

    if (task_map.count(currentEdge[1u].asString()) == 0)
      throw std::runtime_error("Edege with operator name " + currentEdge[1u].asString() + " not found");

    std::string left = currentEdge[0u].asString();
    std::string right = currentEdge[1u].asString();

    std::shared_ptr<Task> src = task_map[left];
    std::shared_ptr<Task> dst = task_map[right];
    if (src != dst) {
      dst->addDependency(src);
    }
  }
}

std::shared_ptr<Task>  QueryParser::getResultTask(
    const task_map_t &task_map) const {
  std::shared_ptr<Task> currentTask, resultTask = nullptr;
  for (auto it = task_map.begin(); it != task_map.end(); ++it) {
    currentTask = it->second;
    // Also, exclude autojson reference table task
    if (!currentTask->hasSuccessors()
        &&  it->first != autojsonReferenceTableId) {
      resultTask = currentTask;
      break;
    }
  }
  return resultTask;
}

std::shared_ptr<PlanOperation> QueryParser::parse(std::string name, const rapidjson::Value& d) {
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
