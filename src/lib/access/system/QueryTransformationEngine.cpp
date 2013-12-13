// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "QueryTransformationEngine.h"
#include <stdexcept>
#include <storage/storage_types.h>


const std::string
QueryTransformationEngine::parallelInstanceInfix = "_instance_",
  QueryTransformationEngine::unionSuffix           = "_union",
  QueryTransformationEngine::mergeSuffix           = "_merge";

Json::Value &QueryTransformationEngine::transform(Json::Value &query) {
  Json::Value::Members operatorIds = query["operators"].getMemberNames();
  Json::Value operatorConfiguration;
  for (size_t i = 0; i < operatorIds.size(); ++i) {
    operatorConfiguration = query["operators"][operatorIds[i]];
    // check whether operator should be transformed; postpone transformation if dynamic transformation is required
    if (operatorConfiguration["dynamic"].asBool() == false && _factory.count(operatorConfiguration["type"].asString()) > 0)
      _factory[operatorConfiguration["type"].asString()]->transform(operatorConfiguration, operatorIds[i], query);
    // check whether operator needs to be parallelized
    if (requestsParallelization(operatorConfiguration))
      applyParallelizationTo(operatorConfiguration, operatorIds[i], query);
  }
  //std::cout << query.toStyledString()<< std::endl;
  return query;
}

bool QueryTransformationEngine::requestsParallelization(
    Json::Value &operatorConfiguration) const {
  const bool parallelize = operatorConfiguration["instances"] >= 2;
  return parallelize;
}

void QueryTransformationEngine::applyParallelizationTo(
    Json::Value &operatorConfiguration,
    const std::string &operatorId,
    Json::Value &query) const {

  std::string consolidateOperatorId;
  Json::Value consolidateOperator;

  if (operatorConfiguration["type"] == "HashBuild") {
    consolidateOperatorId = mergeIdFor(operatorId);
    consolidateOperator = this->mergeOperator(operatorConfiguration["key"].asString());
  } else {
    consolidateOperatorId = unionIdFor(operatorId);
    consolidateOperator = this->unionOperator();
  }
  query["operators"][consolidateOperatorId] = consolidateOperator;
  const size_t numberOfInitialEdges = query["edges"].size();

  std::vector<std::string> *instanceIds = buildInstances(
        query, operatorConfiguration, operatorId, consolidateOperatorId);
    replaceOperatorWithInstances(
        operatorId, *instanceIds, consolidateOperatorId, query, numberOfInitialEdges);
  delete instanceIds;
  
}

std::vector<std::string> *QueryTransformationEngine::buildInstances(
    Json::Value &query,
    Json::Value &operatorConfiguration,
    const std::string &operatorId,
    const std::string &consolidateOperatorId) const {
  const size_t numberOfCores = operatorConfiguration["cores"].size();
  const size_t numberOfInstances = operatorConfiguration["instances"].asInt();
  std::vector<std::string> *instanceIds = new std::vector<std::string>;
  instanceIds->reserve(numberOfInstances);
  for (size_t i = 0; i < numberOfInstances; ++i) {
    std::string nextInstanceId = instanceIdFor(operatorId, i);
    Json::Value nextInstance = nextInstanceOf(
        operatorConfiguration, operatorId, i, numberOfInstances, numberOfCores);
    query["operators"][nextInstanceId] = nextInstance;
    instanceIds->push_back(nextInstanceId);
  }
  return instanceIds;
}

Json::Value QueryTransformationEngine::nextInstanceOf(
    Json::Value &operatorConfiguration,
    const std::string &operatorId,
    const size_t instanceId,
    const size_t numberOfInstances,
    const size_t numberOfCores) const {
  Json::Value nextInstance(operatorConfiguration);
  nextInstance["instances"] = 1;
  nextInstance["part"] = Json::Value((Json::UInt) instanceId);
  nextInstance["count"] = Json::Value((Json::UInt) numberOfInstances);
  if (numberOfCores > 0) {
    nextInstance["core"] = operatorConfiguration["cores"][(int)(instanceId % numberOfCores)];
  }
  return nextInstance;
}

Json::Value QueryTransformationEngine::unionOperator() const {
  Json::Value unionOperator(Json::objectValue);
  unionOperator["type"] = "UnionAll";
  unionOperator["positions"] = true;
  return unionOperator;
}

Json::Value QueryTransformationEngine::mergeOperator(const std::string &key) const {
  Json::Value mergeOperator(Json::objectValue);
  mergeOperator["type"] = "MergeHashTables";
  mergeOperator["positions"] = true;
  mergeOperator["key"] = key;
  return mergeOperator;
}

void QueryTransformationEngine::replaceOperatorWithInstances(
    const std::string &operatorId,
    const std::vector<std::string> &instanceIds,
    const std::string &consolidateOperatorId,
    Json::Value &query,
    const size_t numberOfInitialEdges) const {
  appendInstancesDstNodeEdges(
      operatorId, instanceIds, query, numberOfInitialEdges);
  appendConsolidateSrcNodeEdges(
      operatorId, instanceIds, consolidateOperatorId, query, numberOfInitialEdges);
  removeOperatorNodes(query, operatorId);
}

void QueryTransformationEngine::appendInstancesDstNodeEdges(
    const std::string &operatorId,
    const std::vector<std::string> &instanceIds,
    Json::Value &query,
    const size_t numberOfInitialEdges) const {
  Json::Value edges = query["edges"];
  for (const auto& instanceId: instanceIds)
      for (unsigned i = 0; i < numberOfInitialEdges; ++i) {
        Json::Value currentEdge = edges[i];
        if (currentEdge[1u].asString() == operatorId)
          appendEdge(currentEdge[0u].asString(), instanceId, query);
      }
}

void QueryTransformationEngine::appendConsolidateSrcNodeEdges(
    const std::string &operatorId,
    const std::vector<std::string> &instanceIds,
    const std::string &consolidateOperatorId,
    Json::Value &query,
    const size_t numberOfInitialEdges) const {
  Json::Value edges = query["edges"];
  for (const auto& instanceId: instanceIds)
      appendEdge(instanceId, consolidateOperatorId, query);

  for (unsigned i = 0; i < numberOfInitialEdges; ++i) {
    Json::Value currentEdge = edges[i];
    if (currentEdge[0u].asString() == operatorId) {
      query["edges"][i][0u] = consolidateOperatorId;
    }
  }
}

void QueryTransformationEngine::removeOperatorNodes(
    Json::Value &query,
    const Json::Value &operatorId) const {
  Json::Value remainingEdges(Json::arrayValue);
  Json::Value jsonOperatorId(operatorId);
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    Json::Value currentEdge = query["edges"][i];
    if (currentEdge[0u] != jsonOperatorId
        &&  currentEdge[1u] != jsonOperatorId) {
      remainingEdges.append(currentEdge);
    }
  }
  query["edges"] = remainingEdges;
  query["operators"].removeMember(operatorId.asString());
}

void QueryTransformationEngine::appendEdge(
    const std::string &srcId,
    const std::string &dstId,
    Json::Value &query) const {
  Json::Value edge(Json::arrayValue);
  edge.append(srcId);
  edge.append(dstId);
  query["edges"].append(edge);
}

std::string QueryTransformationEngine::instanceIdFor(
    const std::string operatorId,
    const size_t instanceId) const {
  std::ostringstream s;
  s << instanceId;
  return operatorId + parallelInstanceInfix + s.str();
}

std::string QueryTransformationEngine::unionIdFor(
    const std::string &operatorId) const {
  return operatorId + unionSuffix;
}

std::string QueryTransformationEngine::mergeIdFor(
    const std::string &operatorId) const {
  return operatorId + mergeSuffix;
}

