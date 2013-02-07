#include "testing/test.h"
#include "access/TableScan.h"
#include "access/pred_EqualsExpression.h"
#include "access/pred_CompoundExpression.h"
#include "io/shortcuts.h"

namespace hyrise { namespace access {

TEST(TableScan, test) {
  auto eq = new EqualsExpression<hyrise_int_t>(0, 0, 1);
  auto tbl = Loader::shortcuts::load("test/tables/companies.tbl");
  TableScan ts(eq);
  ts.addInput(tbl);
  const auto& result = ts.execute()->getResultTable();
  ASSERT_EQ(1u, result->size());
}

TEST(TableScan, test2) {
  auto eq = new EqualsExpression<hyrise_string_t>(0, 1, "Apple Inc");
  auto tbl = Loader::shortcuts::load("test/tables/companies.tbl");
  TableScan ts(eq);
  ts.addInput(tbl);
  const auto& result = ts.execute()->getResultTable();
  ASSERT_EQ(1u, result->size());
}

}}
