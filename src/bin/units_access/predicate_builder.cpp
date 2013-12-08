// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <access.h>
#include <io.h>
#include <io/shortcuts.h>
#include <storage.h>
#include <helper/types.h>

#include "helper.h"
#include <json.h>

namespace hyrise {
namespace access {

class PredicateBldr : public AccessTest {};

TEST_F(PredicateBldr, one_field) {
  auto e = new EqualsExpression<hyrise_int_t>((size_t)0, 0, 2009);

  PredicateBuilder b;
  b.add(e);

  SimpleExpression *r = b.build();

  ASSERT_TRUE(r == e);

  delete e;
}

TEST_F(PredicateBldr, one_leg_expression) {
  auto c = new CompoundExpression(NOT);
  auto e = new EqualsExpression<hyrise_int_t>((size_t)0, 0, 2009);

  PredicateBuilder b;
  b.add(c);
  b.add(e);

  SimpleExpression *r = b.build();
  ASSERT_TRUE(r == c);

  ASSERT_EQ(((CompoundExpression *) r)->lhs, e);

}

TEST_F(PredicateBldr, complex_expression) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  auto expr1 = new EqualsExpression<hyrise_int_t>(t, 0, 2009);
  auto expr2 = new EqualsExpression<hyrise_int_t>(t, 1, 1);
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

}
}

