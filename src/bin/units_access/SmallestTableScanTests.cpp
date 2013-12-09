// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SmallestTableScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class SmallestTableScanTests : public AccessTest {};

TEST_F(SmallestTableScanTests, basic_smallest_table_scan_test) {
  auto t1 = io::Loader::shortcuts::load("test/10_30_group.tbl");
  auto t2 = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto t3 = io::Loader::shortcuts::load("test/lin_xxxs.tbl");

  SmallestTableScan stc;
  stc.addInput(t1);
  stc.addInput(t2);
  stc.addInput(t3);
  stc.execute();

  const auto &result = stc.getResultTable();

  ASSERT_TABLE_EQUAL(result, t3);
}

}
}
