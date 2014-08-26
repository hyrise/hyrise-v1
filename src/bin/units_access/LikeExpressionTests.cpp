// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleTableScan.h"
#include "access/expressions/predicates.h"
#include "access/UnionAll.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class LikeExpressionTests : public AccessTest {};

TEST_F(LikeExpressionTests, basic_like_expression_test) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/students.tbl");
  auto expr1 = new LikeExpression(t, 0, ".*ger");
  auto expr2 = new LikeExpression(t, 0, "...iel.*");
  auto expr3 = new LikeExpression(t, 2, ".*o.*a.*");

  SimpleTableScan sts1;
  sts1.addInput(t);
  sts1.setPredicate(expr1);
  sts1.execute();

  SimpleTableScan sts2;
  sts2.addInput(t);
  sts2.setPredicate(expr2);
  sts2.execute();

  SimpleTableScan sts3;
  sts3.addInput(t);
  sts3.setPredicate(expr3);
  sts3.execute();
  const auto& result1 = sts1.getResultTable();
  const auto& result2 = sts2.getResultTable();
  const auto& result3 = sts3.getResultTable();

  ASSERT_EQ(2u, result1->size());
  ASSERT_EQ(703585, result1->getValue<storage::hyrise_int_t>(1, 0));
  ASSERT_EQ(703625, result1->getValue<storage::hyrise_int_t>(1, 1));

  ASSERT_EQ(3u, result2->size());
  ASSERT_EQ(703572, result2->getValue<storage::hyrise_int_t>(1, 0));

  ASSERT_EQ(25u, result3->size());
  ASSERT_EQ(703568, result3->getValue<storage::hyrise_int_t>(1, 0));
  ASSERT_EQ(703623, result3->getValue<storage::hyrise_int_t>(1, 24));
}

TEST_F(LikeExpressionTests, all_and_empty_like_scan) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/students.tbl");
  auto expr1 = new LikeExpression(t, 0, ".*");
  auto expr2 = new LikeExpression(t, 0, "");

  SimpleTableScan sts1;
  sts1.addInput(t);
  sts1.setPredicate(expr1);
  sts1.execute();

  SimpleTableScan sts2;
  sts2.addInput(t);
  sts2.setPredicate(expr2);
  sts2.execute();

  const auto& result1 = sts1.getResultTable();
  const auto& result2 = sts2.getResultTable();

  ASSERT_EQ(59u, result1->size());

  ASSERT_EQ(0u, result2->size());
}
}
}
