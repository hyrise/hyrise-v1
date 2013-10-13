// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"
#include "helper/Settings.h"
#include <json.h>
#include <access.h>
#include <io.h>
#include <storage.h>

#include <io/shortcuts.h>
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace access {

class JSONTests : public AccessTest {};


TEST_F(JSONTests, apply_operator_parallelization) {
  std::string
      parOperatorId = "0",
      instanceId0 = "0_instance_0",
      instanceId1 = "0_instance_1",
      dstNodeId = "1";
  Json::Value query(Json::objectValue);
  Json::Value parOperator(Json::objectValue);
  parOperator["instances"] = 2;
  query["operators"][parOperatorId] = parOperator;
  query["edges"] = EdgesBuilder().
      appendEdge(parOperatorId, dstNodeId).
      getEdges();

  QueryTransformationEngine::getInstance()->applyParallelizationTo(
      parOperator, parOperatorId, query);
  ASSERT_TRUE(query["operators"].getMemberNames().size() == 3);
  ASSERT_TRUE(query["operators"].isMember(parOperatorId) == false);
  ASSERT_TRUE(query["operators"].isMember(instanceId0));
  ASSERT_TRUE(query["operators"][instanceId0]["part"].asInt() == 0);
  ASSERT_TRUE(query["operators"][instanceId0]["count"].asInt() == 2);
  ASSERT_TRUE(query["operators"].isMember(instanceId1));
  ASSERT_TRUE(query["operators"][instanceId1]["part"].asInt() == 1);
  ASSERT_TRUE(query["operators"][instanceId1]["count"].asInt() == 2);
}

TEST_F(JSONTests, operator_replacement) {
  std::string
      nodeId = "0",
      instanceId1 = "1",
      instanceId2 = "2",
      unionId = "u",
      whatever = "whatever";
  std::vector<std::string> instanceIds;
  instanceIds.push_back(instanceId1);
  instanceIds.push_back(instanceId2);
  size_t numberOfInitialEdges = 2;
  Json::Value query(Json::objectValue);
  query["edges"] = EdgesBuilder().
      appendEdge(nodeId, whatever).
      appendEdge(whatever, nodeId).
      getEdges();

  QueryTransformationEngine::getInstance()->replaceOperatorWithInstances(
      nodeId, instanceIds, unionId, query, numberOfInitialEdges);
  ASSERT_TRUE(isEdgeEqual(query["edges"], 0, whatever, instanceId1));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 1, whatever, instanceId2));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 2, instanceId1, unionId));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 3, instanceId2, unionId));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 4, unionId, whatever));
}

TEST_F(JSONTests, append_instances_nodes) {
  std::string
      srcNode = "0",
      dstNode = "1";
  std::string
      dstNodeInstance1 = QueryTransformationEngine::getInstance()->
      instanceIdFor(dstNode, 0),
      dstNodeInstance2 = QueryTransformationEngine::getInstance()->
      instanceIdFor(dstNode, 1);
  std::vector<std::string> instanceIds;
  instanceIds.push_back(dstNodeInstance1);
  instanceIds.push_back(dstNodeInstance2);
  Json::Value query(Json::objectValue);
  query["edges"] = EdgesBuilder().
      appendEdge(srcNode, dstNode).
      getEdges();

  QueryTransformationEngine::getInstance()->appendInstancesDstNodeEdges(
      dstNode, instanceIds, query, 1);
  ASSERT_TRUE(isEdgeEqual(query["edges"], 0, srcNode, dstNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 1, srcNode, dstNodeInstance1));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 2, srcNode, dstNodeInstance2));
}

TEST_F(JSONTests, append_merge_node) {
  std::string
      srcNode = "0",
      dstNode = "1";
  std::string
      srcNodeInstance1 = QueryTransformationEngine::getInstance()->
      instanceIdFor(srcNode, 0),
      srcNodeInstance2 = QueryTransformationEngine::getInstance()->
      instanceIdFor(srcNode, 1),
      srcMergeNode = QueryTransformationEngine::getInstance()->
      mergeIdFor(srcNode);
  std::vector<std::string> instanceIds;
  instanceIds.push_back(srcNodeInstance1);
  instanceIds.push_back(srcNodeInstance2);
  Json::Value query(Json::objectValue);
  query["edges"] = EdgesBuilder().
      appendEdge(srcNode, dstNode).
      getEdges();

  QueryTransformationEngine::getInstance()->appendConsolidateSrcNodeEdges(
      srcNode, instanceIds, srcMergeNode, query, 1);
  ASSERT_TRUE(isEdgeEqual(query["edges"], 0, srcNode, dstNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 1, srcNodeInstance1, srcMergeNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 2, srcNodeInstance2, srcMergeNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 3, srcMergeNode, dstNode));
}

TEST_F(JSONTests, append_union_node) {
  std::string
      srcNode = "0",
      dstNode = "1";
  std::string
      srcNodeInstance1 = QueryTransformationEngine::getInstance()->
      instanceIdFor(srcNode, 0),
      srcNodeInstance2 = QueryTransformationEngine::getInstance()->
      instanceIdFor(srcNode, 1),
      srcUnionNode = QueryTransformationEngine::getInstance()->
      unionIdFor(srcNode);
  std::vector<std::string> instanceIds;
  instanceIds.push_back(srcNodeInstance1);
  instanceIds.push_back(srcNodeInstance2);
  Json::Value query(Json::objectValue);
  query["edges"] = EdgesBuilder().
      appendEdge(srcNode, dstNode).
      getEdges();

  QueryTransformationEngine::getInstance()->appendConsolidateSrcNodeEdges(
      srcNode, instanceIds, srcUnionNode, query, 1);
  ASSERT_TRUE(isEdgeEqual(query["edges"], 0, srcNode, dstNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 1, srcNodeInstance1, srcUnionNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 2, srcNodeInstance2, srcUnionNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 3, srcUnionNode, dstNode));
}

TEST_F(JSONTests, remove_operator_nodes) {
  std::string
      operatorToRemove = "0",
      someNode = "1";
  Json::Value query(Json::objectValue);
  query["edges"] = EdgesBuilder().
      appendEdge(operatorToRemove, someNode).
      appendEdge(someNode, someNode).
      appendEdge(someNode, operatorToRemove).
      appendEdge(someNode, someNode).
      getEdges();

  QueryTransformationEngine::getInstance()->removeOperatorNodes(
      query, operatorToRemove);
  ASSERT_TRUE(isEdgeEqual(query["edges"], 0, someNode, someNode));
  ASSERT_TRUE(isEdgeEqual(query["edges"], 1, someNode, someNode));
}

TEST_F(JSONTests, simple_parse) {
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  bool parsingSuccessful = reader.parse("{\"test\": 1}", root);
  ASSERT_TRUE(parsingSuccessful);
}

TEST_F(JSONTests, parse_projection) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/simple_projection.tbl");
  std::string query = "{\"type\": \"ProjectionScan\", \"fields\": [0], \"inputs\": [\"table1\"] }";

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(query , root);

  ASSERT_TRUE(parsingSuccessful);

  // Create the Plan Op

  auto ps = std::dynamic_pointer_cast<PlanOperation>(QueryParser::instance().parse("ProjectionScan", root));
  ps->addInput(t);
  ps->execute();
  ASSERT_EQ(OpSuccess, ps->getState());
  auto result = ps->getResultTable();
  ASSERT_TRUE(result->contentEquals(reference));

}

TEST_F(JSONTests, parse_papi_event_set) {
  StorageManager::getInstance()->loadTableFile("lin_xxs", "lin_xxs.tbl");
  StorageManager::getInstance()->loadTableFile("lin_xxs_comp", "reference/simple_projection.tbl");

  std::string q = loadFromFile("test/json/simple_query_with_papi.json");

  std::string papi;
  const auto& out = executeAndWait(q, nullptr, 1, &papi);

  ASSERT_FALSE(!out);
  ASSERT_TABLE_EQUAL(out, StorageManager::getInstance()->getTable("lin_xxs_comp"));
  ASSERT_EQ("PAPI_L2_DCM", papi);
  StorageManager::getInstance()->removeAll();
}

#ifdef USE_PAPI_TRACE
TEST_F(JSONTests, parse_papi_badevent_set) {
  StorageManager::getInstance()->loadTableFile("lin_xxs", "lin_xxs.tbl");
  StorageManager::getInstance()->loadTableFile("lin_xxs_comp", "reference/simple_projection.tbl");

  std::string q = loadFromFile("test/json/simple_query_with_papi_bad.json");

  std::string papi;
  ASSERT_THROW( {
      executeAndWait(q, nullptr, 1, &papi);
    }, std::runtime_error);

  StorageManager::getInstance()->removeAll();
}
#endif

TEST_F(JSONTests, parse_papi_event_not_set) {
  StorageManager::getInstance()->loadTableFile("lin_xxs", "lin_xxs.tbl");
  StorageManager::getInstance()->loadTableFile("lin_xxs_comp", "reference/simple_projection.tbl");

  std::string q = loadFromFile("test/json/simple_query.json");

  std::string papi;
  const auto& out = executeAndWait(q, nullptr, 1, &papi);

  ASSERT_FALSE(!out);
  ASSERT_TABLE_EQUAL(out, StorageManager::getInstance()->getTable("lin_xxs_comp"));
  ASSERT_EQ("PAPI_TOT_INS", papi);
  StorageManager::getInstance()->removeAll();

}

TEST_F(JSONTests, parse_predicate) {
  std::string p = "{\"type\": 0, \"in\":0, \"f\":1, \"vtype\":0, \"value\":9989}";
  Json::Value predicate;
  Json::Reader reader;
  ASSERT_TRUE(reader.parse(p, predicate));
  EqualsExpression<hyrise_int_t> *e = (EqualsExpression<hyrise_int_t> *) buildFieldExpression(PredicateType::EqualsExpression, predicate);
  ASSERT_EQ(e->value, 9989);
  delete e;
}

TEST_F(JSONTests, parse_predicate_string) {
  std::string p = "{\"type\": 0, \"in\":0, \"f\":1, \"vtype\":2, \"value\":\"9989\"}";
  Json::Value predicate;
  Json::Reader reader;
  ASSERT_TRUE(reader.parse(p, predicate));
  EqualsExpression<std::string> *e = (EqualsExpression<std::string> *) buildFieldExpression(PredicateType::EqualsExpression, predicate);
  ASSERT_EQ(e->value, "9989");
  delete e;
}


TEST_F(JSONTests, parse_selection) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/groupby_xs.tbl");
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/simple_select_1.tbl");

  std::string query = "{\"type\": \"SimpleTableScan\", \"predicates\":[{\"type\": 8},{\"type\": 7},{\"type\": 0, \"in\":0, \"f\":0, \"vtype\":0, \"value\":2009},{\"type\": 0, \"in\":0, \"f\":1, \"vtype\":0, \"value\":1}]}";

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(query , root);

  ASSERT_TRUE(parsingSuccessful);

  // Create the Plan Op
  auto pop = std::dynamic_pointer_cast<PlanOperation>(QueryParser::instance().parse("SimpleTableScan", root));

  pop->addInput(t);

  const auto& out = pop->execute()->getResultTable();
  ASSERT_TABLE_EQUAL(out, reference);
}

TEST_F(JSONTests, simple_query_parser) {
  StorageManager::getInstance()->loadTableFile("lin_xxs", "lin_xxs.tbl");
  StorageManager::getInstance()->loadTableFile("lin_xxs_comp", "reference/simple_projection.tbl");

  std::string q = loadFromFile("test/json/simple_query.json");

  const auto& out = executeAndWait(q);

  ASSERT_FALSE(!out);

  ASSERT_TABLE_EQUAL(out, StorageManager::getInstance()->getTable("lin_xxs_comp"));
  StorageManager::getInstance()->removeAll();
}

TEST_F(JSONTests, simple_query_with_names_parser) {
  StorageManager::getInstance()->loadTableFile("lin_xxs", "lin_xxs.tbl");
  StorageManager::getInstance()->loadTableFile("lin_xxs_comp", "reference/simple_projection.tbl");

  std::string q = loadFromFile("test/json/simple_query_with_names.json");

  const auto& out = executeAndWait(q);

  ASSERT_FALSE(!out);

  ASSERT_TABLE_EQUAL(out, StorageManager::getInstance()->getTable("lin_xxs_comp"));
  StorageManager::getInstance()->removeAll();
}

TEST_F(JSONTests, DISABLED_group_by_parser) {
  StorageManager::getInstance()->loadTableFile("groupby", "10_30_group.tbl");
  StorageManager::getInstance()->loadTableFile("reference", "reference/group_by_scan_with_sum.tbl");

  std::string q = loadFromFile("test/json/group_by_query.json");

  const auto& out = executeAndWait(q);

  ASSERT_FALSE(!out);

  ASSERT_TABLE_EQUAL(out, StorageManager::getInstance()->getTable("reference"));
  StorageManager::getInstance()->removeAll();
}

TEST_F(JSONTests, complex_query_parser) {

  StorageManager::getInstance()->loadTableFile("groupby_xs", "groupby_xs.tbl");
  StorageManager::getInstance()->loadTableFile("reference", "reference/simple_select_1.tbl");

  std::string q = loadFromFile("test/json/complex_query.json");

  const auto& out = executeAndWait(q);

  ASSERT_FALSE(!out);

  ASSERT_TABLE_EQUAL(out, StorageManager::getInstance()->getTable("reference"));
  StorageManager::getInstance()->removeAll();
}

TEST_F(JSONTests, edges_query_parser) {
  StorageManager::getInstance()->loadTableFile("reference", "edges_ref.tbl");


  std::string query = loadFromFile("test/json/edges_query.json");
  const auto& result = executeAndWait(query);
  ASSERT_FALSE(!result);
  ASSERT_TABLE_EQUAL(result, StorageManager::getInstance()->getTable("reference"));
  StorageManager::getInstance()->removeAll();
}

TEST_F(JSONTests, parallel_query_positions_parser) {
  StorageManager::getInstance()->loadTableFile("reference", "edges_ref.tbl");
  //std::string query = loadFromFile("test/json/parallel_query_positions.json");
  std::string query = loadFromFile("test/json/parallel_stc_with_join.json");

  const auto& result = executeAndWait(query, nullptr, 4);
  ASSERT_FALSE(!result);

  //ASSERT_TABLE_EQUAL(result, StorageManager::getInstance()->getTable("reference"));
  StorageManager::getInstance()->removeAll();
}

TEST_F(JSONTests, parallel_query_materializing_parser) {
  StorageManager::getInstance()->loadTableFile("reference", "edges_ref.tbl");
  std::string query = loadFromFile("test/json/parallel_query_materializing.json");

  const auto& result = executeAndWait(query, nullptr, 4);
  ASSERT_FALSE(!result);

  ASSERT_TABLE_EQUAL(result, StorageManager::getInstance()->getTable("reference"));
  StorageManager::getInstance()->removeAll();
}

}
}

