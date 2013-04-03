// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleTableScan.h"
#include "access/predicates.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class SimpleTableScanTests : public AccessTest {};

TEST_F(SimpleTableScanTests, basic_simple_table_scan_test) {
  storage::c_atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");
  auto expr = new EqualsExpression<storage::hyrise_int_t>(t, 0, 100);

  SimpleTableScan sts;
  sts.addInput(t);
  sts.setPredicate(expr);
  sts.execute();

  const auto &result = sts.getResultTable();

  ASSERT_EQ(1u, result->size());
  ASSERT_EQ(100, result->getValue<storage::hyrise_int_t>(0, 0));
}

}
}
