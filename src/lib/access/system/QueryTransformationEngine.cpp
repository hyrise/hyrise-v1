// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "QueryTransformationEngine.h"
#include <stdexcept>
#include <storage/storage_types.h>


const std::string
QueryTransformationEngine::parallelInstanceInfix = "_instance_",
  QueryTransformationEngine::unionSuffix           = "_union",
  QueryTransformationEngine::mergeSuffix           = "_merge";

rapidjson::Document &QueryTransformationEngine::transform(rapidjson::Document &query) {

  const auto& operatorIds = query["operators"].getMemberNames();
  for (size_t i = 0; i < operatorIds.size(); ++i) {
    const auto& operatorConfiguration = query["operators"][operatorIds[i]];
    // check whether operator should be transformed
    if (_factory.count(operatorConfiguration["type"].asString()) > 0)
      _factory[operatorConfiguration["type"].asString()]->transform(operatorConfiguration, operatorIds[i], query);
    // check whether operator needs to be parallelized
    if (requestsParallelization(operatorConfiguration))
      applyParallelizationTo(operatorConfiguration, operatorIds[i], query);
  }
  //std::cout << query.toStyledString()<< std::endl;
  return query;
}

bool QueryTransformationEngine::requestsParallelization(
    const rapidjson::Value &operatorConfiguration) const {
  const bool parallelize = operatorConfiguration["instances"].asUInt() >= 2;
  return parallelize;
}

void QueryTransformationEngine::applyParallelizationTo(
    const rapidjson::Value &operatorConfiguration,
    const std::string &operatorId,
    rapidjson::Document &query) const {

  std::string consolidateOperatorId;

  if (operatorConfiguration["type"].asString() == "HashBuild") {
    consolidateOperatorId = mergeIdFor(operatorId);
    rapidjson::Value&& o = mergeOperator(query, operatorConfiguration["key"].asString());
    query["operators"].AddMember(consolidateOperatorId.c_str(), o , query.GetAllocator());
  } else {
    consolidateOperatorId = unionIdFor(operatorId);
    rapidjson::Value&& o = unionOperator(query);
    query["operators"].AddMember(consolidateOperatorId.c_str(), o, query.GetAllocator());
  }
  const size_t numberOfInitialEdges = query["edges"].size();

  std::vector<std::string> *instanceIds = buildInstances(query, operatorConfiguration, operatorId, consolidateOperatorId);
  replaceOperatorWithInstances(operatorId, *instanceIds, consolidateOperatorId, query, numberOfInitialEdges);
  delete instanceIds;
  
}

std::vector<std::string> *QueryTransformationEngine::buildInstances(
    rapidjson::Document &query,
    const rapidjson::Value &operatorConfiguration,
    const std::string &operatorId,
    const std::string &consolidateOperatorId) const {

  const size_t numberOfCores = operatorConfiguration["cores"].size();
  const size_t numberOfInstances = operatorConfiguration["instances"].asInt();
  std::vector<std::string> *instanceIds = new std::vector<std::string>;
  instanceIds->reserve(numberOfInstances);
  for (size_t i = 0; i < numberOfInstances; ++i) {
    std::string nextInstanceId = instanceIdFor(operatorId, i);

    rapidjson::Value&& nextInstance = nextInstanceOf(
        query, operatorConfiguration, operatorId, i, numberOfInstances, numberOfCores);

    query["operators"].AddMember(nextInstanceId.c_str(), nextInstance, query.GetAllocator());
    instanceIds->push_back(nextInstanceId);
  }
  return instanceIds;
}

rapidjson::Value&& QueryTransformationEngine::nextInstanceOf(
    rapidjson::Document& doc,
    const rapidjson::Value &operatorConfiguration,
    const std::string &operatorId,
    const size_t instanceId,
    const size_t numberOfInstances,
    const size_t numberOfCores) const {

  // New instance based on a copy
  rapidjson::Document nextInstance;
  operatorConfiguration.Accept(nextInstance);
  
  nextInstance.AddMember("instances", 1, doc.GetAllocator());
  nextInstance.AddMember("part", instanceId, doc.GetAllocator());
  nextInstance.AddMember("count", numberOfInstances, doc.GetAllocator());
  
  if (numberOfCores > 0) {
    nextInstance.AddMember("core", operatorConfiguration["cores"][(int)(instanceId % numberOfCores)].asUInt(), doc.GetAllocator());
  }
  return std::move(nextInstance);
}

rapidjson::Value&& QueryTransformationEngine::unionOperator(rapidjson::Document& d) const {
  rapidjson::Value unionOperator(rapidjson::kObjectType);
  unionOperator.AddMember("type", "UnionAll", d.GetAllocator());
  unionOperator.AddMember("positions", "true", d.GetAllocator());
  return std::move(unionOperator);
}

rapidjson::Value&& QueryTransformationEngine::mergeOperator(rapidjson::Document& d,const std::string &key) const {
  rapidjson::Value mergeOperator(rapidjson::kObjectType);
  mergeOperator.AddMember("type", "MergeHashTables", d.GetAllocator());
  mergeOperator.AddMember("positions", "true", d.GetAllocator());
  mergeOperator.AddMember("key", key, d.GetAllocator());
  return std::move(mergeOperator);
}

void QueryTransformationEngine::replaceOperatorWithInstances(
    const std::string &operatorId,
    const std::vector<std::string> &instanceIds,
    const std::string &consolidateOperatorId,
    rapidjson::Document &query,
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
    rapidjson::Document &query,
    const size_t numberOfInitialEdges) const {
  const rapidjson::Value& edges = query["edges"];
  for (const auto& instanceId: instanceIds)
      for (unsigned i = 0; i < numberOfInitialEdges; ++i) {
        const rapidjson::Value& currentEdge = edges[i];
        if (currentEdge[1u].asString() == operatorId)
          appendEdge(currentEdge[0u].asString(), instanceId, query);
      }
}

void QueryTransformationEngine::appendConsolidateSrcNodeEdges(
    const std::string &operatorId,
    const std::vector<std::string> &instanceIds,
    const std::string &consolidateOperatorId,
    rapidjson::Document &query,
    const size_t numberOfInitialEdges) const {
  const rapidjson::Value& edges = query["edges"];
  for (const auto& instanceId: instanceIds)
      appendEdge(instanceId, consolidateOperatorId, query);

  for (unsigned i = 0; i < numberOfInitialEdges; ++i) {
    const rapidjson::Value& currentEdge = edges[i];
    if (currentEdge[0u].asString() == operatorId) {
      appendEdge(consolidateOperatorId, currentEdge[1u].asString(), query);
    }
  }
}

void QueryTransformationEngine::removeOperatorNodes(
    rapidjson::Document &query,
    const rapidjson::Value &operatorId) const {
  rapidjson::Value remainingEdges(rapidjson::kArrayType);

  for (unsigned i = 0; i < query["edges"].size(); ++i) {

    // Copy the edge
    rapidjson::Document currentEdge;
    query["edges"][i].Accept(currentEdge);

    if (currentEdge[0u].asString() != operatorId.asString()
        &&  currentEdge[1u].asString() != operatorId.asString()) {
      remainingEdges.PushBack(currentEdge, query.GetAllocator());
    }
  }

  //query["edges"] = remainingEdges;
  query.AddMember("edges", remainingEdges, query.GetAllocator());
  query["operators"].RemoveMember(operatorId.asString().c_str());
}

void QueryTransformationEngine::appendEdge(
    const std::string &srcId,
    const std::string &dstId,
    rapidjson::Document &query) const {

  rapidjson::Value edge(rapidjson::kArrayType);
  edge.PushBack(srcId, query.GetAllocator());
  edge.PushBack(dstId, query.GetAllocator());
  query["edges"].PushBack(edge, query.GetAllocator());
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

