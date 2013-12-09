// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleTableScan.h"
#include "access/expressions/predicates.h"
#include "access/UnionAll.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class SimpleTableScanTests : public AccessTest {};

TEST_F(SimpleTableScanTests, basic_simple_table_scan_test) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto expr = new EqualsExpression<storage::hyrise_int_t>(t, 0, 100);

  SimpleTableScan sts;
  sts.addInput(t);
  sts.setPredicate(expr);
  sts.execute();

  const auto &result = sts.getResultTable();

  ASSERT_EQ(1u, result->size());
  ASSERT_EQ(100, result->getValue<storage::hyrise_int_t>(0, 0));
}

// Same as above, but manually parallelized
TEST_F(SimpleTableScanTests, parallelized_simple_table_scan) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");

  SimpleTableScan sts1;
  sts1.addInput(t);
  sts1.setPredicate(new EqualsExpression<storage::hyrise_int_t>(0, 0, 100));
  sts1.setPart(0);
  sts1.setCount(2);
  sts1.execute();

  SimpleTableScan sts2;
  sts2.addInput(t);
  sts2.setPredicate(new EqualsExpression<storage::hyrise_int_t>(0, 0, 100));
  sts2.setPart(1);
  sts2.setCount(2);
  sts2.execute();

  auto result1 = sts1.getResultTable();
  auto result2 = sts2.getResultTable();

  UnionAll ua;
  ua.addInput(result1);
  ua.addInput(result2);
  ua.execute();

  auto result = ua.getResultTable();

  ASSERT_EQ(1u, result->size());
  ASSERT_EQ(100, result->getValue<storage::hyrise_int_t>(0, 0));
}

}
}
