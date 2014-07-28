// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include "access/SimpleTableScan.h"
#include "access/expressions/predicates.h"
#include "io/shortcuts.h"
#include "storage/HorizontalTable.h"

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

TEST_F(PredicateBldr, complex_expression_matchall) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");
  storage::c_atable_ptr_t two =
      std::make_shared<const storage::HorizontalTable>(std::vector<storage::c_atable_ptr_t>{t, t});
  {
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
  {
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

  {
    auto expr3 = new CompoundExpression(AND);
    auto expr1 = new EqualsExpression<hyrise_int_t>(two, 0, 2009);
    auto expr2 = new EqualsExpression<hyrise_int_t>(two, 1, 7);

    PredicateBuilder b;
    b.add(expr3);
    b.add(expr2);
    b.add(expr1);

    auto p = b.build();
    p->walk({two});
    std::cout << two->size() << "two size " << std::endl;
    auto x = p->matchAll(0, two->size());
    EXPECT_EQ(6, x.size());
  }
}
}
}
