// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_QUERYTRANSFORMATIONENGINE_H_
#define SRC_LIB_ACCESS_QUERYTRANSFORMATIONENGINE_H_

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <json.h>
#include <memory>
#include "access/system/AbstractPlanOpTransformation.h"

namespace hyrise {
namespace access {
class JSONTests_operator_replacement_Test;
class JSONTests_apply_operator_parallelization_Test;
class JSONTests_append_instances_nodes_Test;
class JSONTests_append_union_node_Test;
class JSONTests_append_merge_node_Test;
class JSONTests_remove_operator_nodes_Test;
}
}

class QueryTransformationEngine {
  friend class hyrise::access::JSONTests_operator_replacement_Test;
  friend class hyrise::access::JSONTests_apply_operator_parallelization_Test;
  friend class hyrise::access::JSONTests_append_instances_nodes_Test;
  friend class hyrise::access::JSONTests_append_union_node_Test;
  friend class hyrise::access::JSONTests_append_merge_node_Test;
  friend class hyrise::access::JSONTests_remove_operator_nodes_Test;

  //  List of affixes for IDs of new or transformed operators.
  static const std::string parallelInstanceInfix;
  static const std::string unionSuffix;
  static const std::string mergeSuffix;


  typedef std::map< std::string, std::unique_ptr<hyrise::access::AbstractPlanOpTransformation> > factory_map_t;
  factory_map_t _factory;

  QueryTransformationEngine() {}

  //  Parallelizes query's operators, if specified.
  void parallelizeOperators(Json::Value &query) const;

  /*  Checks if given operator of given configuration is meant to be executed
      in parallel. */
  bool requestsParallelization(Json::Value &operatorConfiguration) const;

  //  The operator will be replaced by its parallel instances in the json query.
  void applyParallelizationTo(
      Json::Value &operatorConfiguration,
      const std::string &operatorId,
      Json::Value &query) const;

  //  Builds an operators parallel instances to arrange them in the query.
  std::vector<std::string> *buildInstances(
      Json::Value &query,
      Json::Value &operatorConfiguration,
      const std::string &operatorId,
      const std::string &unionOperatorId) const;

  /*  Constructs the next instance's configuration for parallel execution of a
      given operator configuration based on the number of total instances. */
  Json::Value nextInstanceOf(
      Json::Value &operatorConfiguration,
      const std::string &operatorId,
      const size_t instanceId,
      const size_t numberOfInstances,
      const size_t numberOfCores) const;

  /*  Constructs the configuration of the union operator merging the parallel
      instances' results. */
  Json::Value unionOperator() const;

    /*  Constructs the configuration of the merge operator merging the parallel
      instances' results. */
  Json::Value mergeOperator(const std::string &key) const;

  /*  Modifies the query plan's edges. The operator to-be-parallelized will be
      replaced by its parallel instances. */
  void replaceOperatorWithInstances(
      const std::string &operatorId,
      const std::vector<std::string> &instanceIds,
      const std::string &unionOperatorId,
      Json::Value &query,
      const size_t numberOfInitialEdges) const;

  /*  For all edges where the original operator is the destination node,
      append a similar edge for each parallel instance. */
  void appendInstancesDstNodeEdges(
      const std::string &operatorId,
      const std::vector<std::string> &instanceIds,
      Json::Value &query,
      const size_t numberOfInitialEdges) const;

  /*  For all edges where the original operator is the source node,
      append a similar edge with the parallelization's final union
      operator. */
  void appendConsolidateSrcNodeEdges(
      const std::string &operatorId,
      const std::vector<std::string> &instanceIds,
      const std::string &unionOperatorId,
      Json::Value &query,
      const size_t numberOfInitialEdges) const;

  /*  Because there is no remove method on a Json array...
      Removes edges containing the original operator. */
  void removeOperatorNodes(
      Json::Value &query,
      const Json::Value &operatorId) const;

  //  Constructs the edge to append to query's edges from given src/dst nodes.
  void appendEdge(
      const std::string &srcId,
      const std::string &dstId,
      Json::Value &query) const;

  //  Construct IDs for several kind of newly inserted operators.
  std::string instanceIdFor(
      const std::string operatorId,
      const size_t instanceId) const;
  std::string unionIdFor(const std::string &operatorId) const;
  std::string mergeIdFor(const std::string &operatorId) const;


 public:
  ~QueryTransformationEngine() {}

  template<typename T>
  static bool registerTransformation() {
    QueryTransformationEngine::getInstance()->_factory[T::name()].reset(new T());
    return true;
  }

  template<typename T>
  static bool registerPlanOperation(const std::string& name) {
    QueryTransformationEngine::getInstance()->_factory[name].reset(new T());
    return true;
  }

  /*  Main method. Transforms a query based on its operators' configurations.
      The resulting query is meant to be directly parsable/executable. */
  Json::Value &transform(Json::Value &query);

  static QueryTransformationEngine *getInstance() {
    static QueryTransformationEngine p;
    return &p;
  }
};

#endif  // SRC_LIB_ACCESS_QUERYTRANSFORMATIONENGINE_H_
