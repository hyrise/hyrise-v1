// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/Distinct.h"
#include "io/shortcuts.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class DistinctTests : public AccessTest {};

TEST_F(DistinctTests, basic_distinct_test) {
  auto t = io::Loader::shortcuts::load("test/tables/employees.tbl");
  auto reference = io::Loader::shortcuts::load("test/tables/employees_distinct.tbl");

  Distinct d;
  d.addInput(t);
  d.addField(1);
  d.execute();

  const auto &result = d.getResultTable();

  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(DistinctTests, distinct_on_distinct_column_test) {
  auto t = io::Loader::shortcuts::load("test/tables/employees.tbl");

  Distinct d;
  d.addInput(t);
  d.addField(0);
  d.execute();

  const auto &result = d.getResultTable();

  EXPECT_RELATION_EQ(result, t);
}

}
}
