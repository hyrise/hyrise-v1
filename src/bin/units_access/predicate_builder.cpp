// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include "access/SimpleTableScan.h"
#include "access/expressions/predicates.h"
#include "access/expressions/pred_EqualsExpression.h"
#include "io/shortcuts.h"
#include "storage/HorizontalTable.h"
#include "storage/Store.h"
#include "storage/debug_table_structure.h"
#include "io/TransactionManager.h"

namespace hyrise {
namespace access {

class PredicateBldr : public AccessTest {};

TEST_F(PredicateBldr, one_field) {
  auto e = new EqualsExpression<hyrise_int_t>((size_t)0, 0, 2009);

  PredicateBuilder b;
  b.add(e);

  SimpleExpression* r = b.build();

  ASSERT_TRUE(r == e);

  delete e;
}

TEST_F(PredicateBldr, one_leg_expression) {
  auto c = new CompoundExpression(NOT);
  auto e = new EqualsExpression<hyrise_int_t>((size_t)0, 0, 2009);

  PredicateBuilder b;
  b.add(c);
  b.add(e);

  SimpleExpression* r = b.build();
  ASSERT_TRUE(r == c);

  ASSERT_EQ(((CompoundExpression*)r)->lhs, e);
}

TEST_F(PredicateBldr, complex_expression) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  auto expr1 = new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(t, 0, 2009);
  auto expr2 = new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(t, 1, 1);
  auto expr3 = new CompoundExpression(OR);
  auto expr4 = new CompoundExpression(NOT);

  PredicateBuilder b;
  b.add(expr4);
  b.add(expr3);
  b.add(expr1);
  b.add(expr2);



  auto scan = std::make_shared<SimpleTableScan>();
  scan->addInput(t);
  scan->setPredicate(b.build());
  scan->setProducesPositions(true);

  auto out = scan->execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_select_1.tbl");

  ASSERT_TRUE(out->contentEquals(reference));
}

TEST(Expression, matchall) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");
  auto expr3 = new CompoundExpression(AND);
  auto expr1 = new EqualsExpression<hyrise_int_t>(t, 0, 2009);
  auto expr2 = new EqualsExpression<hyrise_int_t>(t, 1, 7);

  PredicateBuilder b;
  b.add(expr3);
  b.add(expr1);
  b.add(expr2);

  auto p = b.build();
  p->walk({t});

  auto x = p->matchAll(0, t->size());
  EXPECT_EQ(x.size(), 3);
}

TEST(Expressions, matchall2) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");


  auto expr3 = new CompoundExpression(AND);
  auto expr1 = new EqualsExpression<hyrise_int_t>(t, 0, 2009);
  auto expr2 = new EqualsExpression<hyrise_int_t>(t, 1, 7);

  PredicateBuilder b;
  b.add(expr3);
  b.add(expr2);
  b.add(expr1);

  auto p = b.build();
  p->walk({t});

  auto x = p->matchAll(0, t->size());
  EXPECT_EQ(x.size(), 3);
}

TEST(Expressions, matchall_less) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  auto expr3 = new CompoundExpression(AND);
  auto expr1 = new EqualsExpression<hyrise_int_t>(t, 0, 2009);
  auto expr2 = new LessExpression<hyrise_int_t>(t, 1, 6);

  PredicateBuilder b;
  b.add(expr3);
  b.add(expr2);
  b.add(expr1);

  auto p = b.build();
  p->walk({t});

  auto x = p->matchAll(0, t->size());
  EXPECT_EQ(x.size(), 7);
}



TEST(Expressions, matchall3) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");
  storage::c_atable_ptr_t two =
      std::make_shared<const storage::HorizontalTable>(std::vector<storage::c_atable_ptr_t>{t, t});

  auto expr3 = new CompoundExpression(AND);
  auto expr1 = new EqualsExpression<hyrise_int_t>(two, 0, 2009);
  auto expr2 = new EqualsExpression<hyrise_int_t>(two, 1, 7);

  PredicateBuilder b;
  b.add(expr3);
  b.add(expr2);
  b.add(expr1);

  auto p = b.build();
  p->walk({two});
  auto x = p->matchAll(0, two->size());
  EXPECT_EQ(6, x.size());
}

TEST(Expressions, matchall4) {
  std::shared_ptr<const storage::AbstractTable> two =
      io::Loader::shortcuts::loadMainDelta("test/groupby_xs.tbl", "test/groupby_xs.tbl");

  auto expr3 = new CompoundExpression(AND);
  auto expr1 = new EqualsExpression<hyrise_int_t>(two, 0, 2009);
  auto expr2 = new EqualsExpression<hyrise_int_t>(two, 1, 7);

  PredicateBuilder b;
  b.add(expr3);
  b.add(expr2);
  b.add(expr1);

  auto p = b.build();
  p->walk({two});
  auto x = p->matchAll(0, two->size());
  EXPECT_EQ(6, x.size());
}

TEST(Expressions, matchall_unordered_input) {
  auto s = checked_pointer_cast<storage::Store>(io::Loader::shortcuts::load("test/groupby_xs.tbl"));
  const auto e = s->size();
  s->appendToDelta(s->size());

  auto ctx = tx::TransactionManager::getInstance().beginTransaction();

  for (size_t i = 0; i < e; ++i) {
    s->copyRowToDelta(s, i, i, ctx.tid);
  }
  auto as_tab = checked_pointer_cast<const storage::AbstractTable>(s);
  auto expr3 = new CompoundExpression(AND);
  auto expr1 = new EqualsExpression<hyrise_int_t>(as_tab, 0, 2009);
  auto expr2 = new EqualsExpression<hyrise_int_t>(as_tab, 1, 7);

  PredicateBuilder b;
  b.add(expr3);
  b.add(expr2);
  b.add(expr1);

  auto p = b.build();
  p->walk({as_tab});
  auto x = p->matchAll(0, s->size());
  EXPECT_EQ(6, x.size());
}
}
}
