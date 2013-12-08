// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/JoinScan.h"
#include "access/SimpleTableScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class JoinScanTests : public AccessTest {};

TEST_F(JoinScanTests, basic_join_scan_test) {
  auto t1 = io::Loader::shortcuts::load("test/join_transactions.tbl");
  auto t2 = io::Loader::shortcuts::load("test/join_exchange.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/join_result.tbl");

  JoinScan js(JoinType::EQUI);
  js.addInput(t1);
  js.addInput(t2);
  js.addCombiningClause(AND);
  js.addJoinClause<int>(0,0,1,0);
  js.addJoinClause<std::string>(0,1,1,1);
  js.execute();

  const auto &result = js.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

}
}
