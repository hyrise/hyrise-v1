// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/UnionScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class UnionScanTests : public AccessTest {};

TEST_F(UnionScanTests, basic_union_scan_test) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  UnionScan us;
  us.addInput(t);
  us.addInput(t);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_EQ(2 * t->size(), result->size());
}

}
}
