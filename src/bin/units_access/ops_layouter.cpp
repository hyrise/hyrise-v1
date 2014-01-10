// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <access.h>

#include "io/StorageManager.h"
#include "io/shortcuts.h"
#include "access/LayoutTable.h"
#include "helper.h"

namespace hyrise {
namespace access {

class LayouterOpsTest : public AccessTest {};

TEST_F(LayouterOpsTest, simple_op) {
  LayoutSingleTable s;
  s.setLayouter(LayoutSingleTable::CandidateLayouter);
  s.setNumRows(10000);
  s.addFieldName("A");
  s.addFieldName("B");
  s.addFieldName("C");

  LayoutSingleTable::BaseQuery q;
  q.positions.push_back(0);
  q.selectivity = 1;
  q.weight = 1;
  s.addQuery(q);

  s.executePlanOperation();
  const auto&  res = s.getResultTable();
  std::string header = loadFromFile("test/header/layouter_simple_cand.tbl");

  ASSERT_EQ(res->getValue<std::string>(0, 0), header);
  ASSERT_EQ(1u, res->size());
}

TEST_F(LayouterOpsTest, simple_op_count) {
  LayoutSingleTable s;
  s.setLayouter(LayoutSingleTable::BaseLayouter);
  s.setNumRows(1000);
  s.addFieldName("A");
  s.addFieldName("B");
  s.addFieldName("C");
  s.setMaxResults(2);

  LayoutSingleTable::BaseQuery q;
  q.positions.push_back(0);
  q.selectivity = 1;
  q.weight = 1;
  s.addQuery(q);

  s.executePlanOperation();
  const auto&  res = s.getResultTable();

  std::string header = loadFromFile("test/header/layouter_simple.tbl");

  ASSERT_EQ(res->getValue<std::string>(0, 0), header);
  ASSERT_EQ(2u, res->size());
}


TEST_F(LayouterOpsTest, simple_parse_json) {
  std::string data = loadFromFile("test/json/simple_layouter.json");
  Json::Value request_data;
  Json::Reader reader;

  // Parse JSON
  reader.parse(data, request_data);

  // parse the json
  auto s = std::dynamic_pointer_cast<LayoutSingleTable>(LayoutSingleTable::parse(request_data["operators"]["0"]));
  s->executePlanOperation();
  const auto& res = s->getResultTable();
  std::string header = loadFromFile("test/header/layouter_simple.tbl");

  ASSERT_EQ(res->getValue<std::string>(0, 0), header);
}

TEST_F(LayouterOpsTest, parse_full_scenario) {
  std::string data = loadFromFile("test/json/simple_layouter.json");
  const auto& e = executeAndWait(data);
  std::string header = loadFromFile("test/header/layouter_simple.tbl");

  ASSERT_EQ(e->getValue<std::string>(0, 0), header);
}

TEST_F(LayouterOpsTest, parse_full_scenario_candidate) {
  std::string data = loadFromFile("test/json/simple_layouter_candidate.json");
  const auto& e = executeAndWait(data);
  std::string header = loadFromFile("test/header/layouter_simple_cand.tbl");

  ASSERT_EQ(e->getValue<std::string>(0, 0), header);
}


TEST_F(LayouterOpsTest, layouting_table_op_basic) {
  auto table = io::Loader::shortcuts::load("test/tables/partitions.tbl");
  ASSERT_EQ(table->partitionCount(), 3u);

  LayoutTable op("a|b|c|d\nINTEGER|INTEGER|INTEGER|INTEGER\n0_C|0_C|1_C|1_C");
  op.addInput(table);
  auto result = op.execute()->getResultTable();

  ASSERT_EQ(result->partitionCount(), 2u);
}

TEST_F(LayouterOpsTest, layouting_table_op_reordering) {
  auto table = io::Loader::shortcuts::load("test/tables/partitions.tbl");
  ASSERT_EQ(table->partitionCount(), 3u);

  LayoutTable op("a|c|b|d\nINTEGER|INTEGER|INTEGER|INTEGER\n0_C|0_C|1_C|1_C");
  op.addInput(table);
  auto result = op.execute()->getResultTable();

  ASSERT_EQ(result->partitionCount(), 2u);
  ASSERT_EQ(result->metadataAt(1).getName(), "c");
  ASSERT_EQ(result->getValue<hyrise_int_t>(1, 0), 3);
}

TEST_F(LayouterOpsTest, load_layout_replace) {
  std::string data = loadFromFile("test/json/load_layout_replace.json");
  const auto& e = executeAndWait(data);
  ASSERT_EQ(e->partitionCount(), 3u);
  ASSERT_EQ(3u, io::StorageManager::getInstance()->getTable("revenue")->partitionCount());
}

}
}

