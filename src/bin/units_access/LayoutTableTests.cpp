// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LayoutTable.h"

#include "io/shortcuts.h"
#include "storage/ColumnMetadata.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class LayoutTableTests : public AccessTest {};

TEST_F(LayoutTableTests, basic_layout_table_test) {
  auto t = io::Loader::shortcuts::load("test/tables/partitions.tbl");
  ASSERT_EQ(3u, t->partitionCount());

  LayoutTable lt("a|c|b|d\nINTEGER|INTEGER|INTEGER|INTEGER\n0_C|0_C|1_C|1_C");
  lt.addInput(t);
  lt.execute();

  const auto &result = lt.getResultTable();

  ASSERT_EQ(2u, result->partitionCount());
  ASSERT_EQ("c", result->metadataAt(1).getName());
  ASSERT_EQ(3, result->getValue<storage::hyrise_int_t>(1, 0));
}

}
}
