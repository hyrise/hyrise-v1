// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "access/TableScan.h"
#include "access/expressions/pred_EqualsExpression.h"
#include "access/expressions/pred_CompoundExpression.h"
#include "io/shortcuts.h"

namespace hyrise { namespace access {

TEST(TableScan, test) {
  auto tbl = Loader::shortcuts::load("test/tables/companies.tbl");
  TableScan ts(std::unique_ptr<EqualsExpression<hyrise_int_t> >(new EqualsExpression<hyrise_int_t>(0, 0, 1)));
  ts.addInput(tbl);
  const auto& result = ts.execute()->getResultTable();
  ASSERT_EQ(1u, result->size());
}

TEST(TableScan, test2) {
  auto eq = std::unique_ptr<EqualsExpression<hyrise_string_t> >(new EqualsExpression<hyrise_string_t>(0, 1, "Apple Inc"));
  auto tbl = Loader::shortcuts::load("test/tables/companies.tbl");
  TableScan ts(std::move(eq));
  ts.addInput(tbl);
  const auto& result = ts.execute()->getResultTable();
  ASSERT_EQ(1u, result->size());
}

}}
