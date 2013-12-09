// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/UnionScan.h"
#include "io/shortcuts.h"
#include "storage/PointerCalculator.h"
#include "testing/test.h"

namespace hyrise { namespace access {

TEST(UnionScanTests, basic_union_scan_test) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto pc = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);

  UnionScan us;
  us.addInput(pc);
  us.addInput(pc);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_EQ(t->size(), result->size());
}

}}
