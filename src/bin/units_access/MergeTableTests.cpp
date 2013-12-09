// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MergeTable.h"
#include "io/shortcuts.h"
#include "storage/Store.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class MergeTableTests : public AccessTest {};

TEST_F(MergeTableTests, basic_merge_table_test) {
  auto s = io::Loader::shortcuts::loadMainDelta("test/merge1_main.tbl", "test/merge1_delta.tbl");
  auto reference = io::Loader::shortcuts::load("test/merge1_result.tbl");

  ASSERT_EQ(4u, s->getMainTable()->size());
  ASSERT_EQ(5u, s->getDeltaTable()->size());

  MergeTable mt;
  mt.addInput(s);
  mt.execute();

  const auto &result = mt.getResultTable();

  ASSERT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

}
}
