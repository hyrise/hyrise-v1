// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MergeTable.h"
#include "io/shortcuts.h"
#include "storage/Store.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class MergeTableTests : public AccessTest {};

TEST_F(MergeTableTests, basic_merge_table_test) {
  auto s = Loader::shortcuts::loadMainDelta("test/merge1_main.tbl", "test/merge1_delta.tbl");
  auto reference = Loader::shortcuts::load("test/merge1_result.tbl");

  ASSERT_EQ((unsigned) 4, s->getMainTables()[0]->size());
  ASSERT_EQ((unsigned) 5, s->getDeltaTable()->size());

  MergeTable mt;
  mt.addInput(s);
  mt.execute();

  const auto &result = mt.getResultTable();

  ASSERT_EQ((unsigned) 9, result->size());
  ASSERT_TRUE(result->contentEquals(reference));
}

}
}
