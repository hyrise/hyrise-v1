// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SortScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class SortScanTests : public AccessTest {};

TEST_F(SortScanTests, basic_sort_scan_test) {
  auto t = io::Loader::shortcuts::load("test/reference/group_by_scan_using_table_2.tbl");
  auto reference = io::Loader::shortcuts::load("test/sort_test.tbl");

  SortScan ss;
  ss.addInput(t);
  ss.setSortField(0);
  ss.execute();

  const auto &result = ss.getResultTable();

  ASSERT_TRUE(result->contentEquals(reference));
}

}
}
