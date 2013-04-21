// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"

#include "io/shortcuts.h"
#include "access/UnionAll.h"

namespace hyrise { namespace access {

TEST(UnionAllTests, basic_union_scan_test) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  UnionAll us;
  us.addInput(t);
  us.addInput(t);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_EQ(2* t->size(), result->size());
}

}
}
