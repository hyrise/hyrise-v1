// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/GroupByScan.h"
#include "access/HashBuild.h"
#include "io/shortcuts.h"
#include "storage/TableEqualityTest.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class GroupByScanTests : public AccessTest {};

TEST_F(GroupByScanTests, basic_group_by_test) {
  auto t = Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();

  const auto &hash = hb.getResultHashTable();

  GroupByScan gs;
  gs.addInput(t);
  gs.addInputHash(hash);
  gs.addField(1);
  gs.execute();

  const auto &result = gs.getResultTable();

  ASSERT_EQ(8u, result->size());
}

TEST_F(GroupByScanTests, group_by_with_multiple_fields) {
  auto t = Loader::shortcuts::load("test/10_30_group.tbl");
  auto reference = Loader::shortcuts::load("test/10_30_group_multi_result.tbl");

  HashBuild hb1;
  hb1.addInput(t);
  hb1.addField(0);
  hb1.setKey("groupby");
  hb1.execute();

  HashBuild hb2;
  hb2.addInput(t);
  hb2.addField(1);
  hb2.setKey("groupby");
  hb2.execute();

  const auto &hash1 = hb1.getResultHashTable();
  const auto &hash2 = hb2.getResultHashTable();

  GroupByScan gs;
  gs.addInput(t);
  gs.addInputHash(hash1);
  gs.addInputHash(hash2);
  gs.addField(0);
  gs.addField(1);
  gs.execute();

  const auto &result = gs.getResultTable();

  EXPECT_RELATION_EQ(reference, result);
}

TEST_F(GroupByScanTests, group_by_with_aggregate_function) {
  auto t = Loader::shortcuts::load("test/10_30_group.tbl");
  auto reference = Loader::shortcuts::load("test/10_30_group_count_result.tbl");

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
  gs.addInputHash(hash);
  gs.addField(1);
  gs.execute();

  const auto &result = gs.getResultTable();

  EXPECT_RELATION_EQ(reference, result);
}

}
}
