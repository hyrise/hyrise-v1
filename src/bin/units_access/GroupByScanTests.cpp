// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/GroupByScan.h"
#include "access/HashBuild.h"
#include "io/shortcuts.h"
#include "testing/TableEqualityTest.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class GroupByScanTests : public AccessTest {};

TEST_F(GroupByScanTests, basic_group_by_test) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();

  const auto &hash = hb.getResultHashTable();

  GroupByScan gs;
  gs.addInput(t);
  gs.addInput(hash);
  gs.addField(1);
  gs.execute();

  const auto &result = gs.getResultTable();

  ASSERT_EQ(8u, result->size());
}

TEST_F(GroupByScanTests, group_by_with_multiple_fields) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");
  auto reference = io::Loader::shortcuts::load("test/10_30_group_multi_result.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(0);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();

  const auto &hash = hb.getResultHashTable();

  GroupByScan gs;
  gs.addInput(t);
  gs.addInput(hash);
  gs.addField(0);
  gs.addField(1);
  gs.execute();

  const auto &result = gs.getResultTable();

  EXPECT_RELATION_EQ(reference, result);
}

TEST_F(GroupByScanTests, group_by_with_aggregate_function) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");
  auto reference = io::Loader::shortcuts::load("test/10_30_group_count_result.tbl");

  auto count = new CountAggregateFun(0);

  HashBuild hb;
  hb.addInput(t);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();

  const auto &hash = hb.getResultHashTable();

  GroupByScan gs;
  gs.addInput(t);
  gs.addFunction(count);
  gs.addInput(hash);
  gs.addField(1);
  gs.execute();

  const auto &result = gs.getResultTable();
  EXPECT_RELATION_EQ(reference, result);
}

}
}
