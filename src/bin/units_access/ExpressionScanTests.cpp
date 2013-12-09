// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ExpressionScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class ExpressionScanTests : public AccessTest {};

TEST_F(ExpressionScanTests, basic_expression_scan_test) {
  auto t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/simple_expression.tbl");

  AddExp plus(t, 0, 1);

  ExpressionScan es;
  es.addInput(t);
  es.setExpression("plus", &plus);
  es.execute();

  const auto &result = es.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

}
}
