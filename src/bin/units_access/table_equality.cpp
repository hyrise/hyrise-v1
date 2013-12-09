// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/shortcuts.h"
#include "storage/AbstractTable.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class TableEqualityTests : public ::testing::Test {};

TEST_F(TableEqualityTests, test_equal) {
  auto t1 = io::Loader::shortcuts::load("test/tables/employees.tbl"),
       t2 = io::Loader::shortcuts::load("test/tables/employees.tbl");
  EXPECT_RELATION_EQ(t1, t2);
}

TEST_F(TableEqualityTests, test_not_equal) {
  auto t1 = io::Loader::shortcuts::load("test/tables/employees.tbl"),
       t2 = io::Loader::shortcuts::load("test/tables/employees_revised.tbl");
  EXPECT_RELATION_NEQ(t1, t2);
}

} } // namespace hyrise::access

