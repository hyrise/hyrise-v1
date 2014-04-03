// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PosUpdateIncrementScan.h"
#include "access/SimpleTableScan.h"
#include "access/TableScan.h"
#include "access/expressions/predicates.h"
#include "access/tx/Commit.h"
#include "access/tx/ValidatePositions.h"
#include "helper/types.h"
#include "io/shortcuts.h"
#include "io/TransactionManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class PosUpdateIncrementTests : public AccessTest {
 public:
  PosUpdateIncrementTests() {}

  virtual void SetUp() {

    hyrise::tx::TransactionManager::getInstance().reset();

    AccessTest::SetUp();
    t = io::Loader::shortcuts::load("test/tables/companies.tbl");
  }

  storage::atable_ptr_t t;
};

TEST_F(PosUpdateIncrementTests, basic_increment_test) {
  auto reference = io::Loader::shortcuts::load("test/reference/companies_after_one_increments.tbl");

  // Increment first
  auto writeCtx = hyrise::tx::TransactionManager::getInstance().buildContext();

  SimpleTableScan sts1;
  sts1.setTXContext(writeCtx);
  sts1.addInput(t);
  sts1.setPredicate(new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(0, 0, 2));
  sts1.execute();

  PosUpdateIncrementScan inc("company_id", 5);
  inc.setTXContext(writeCtx);
  inc.addInput(sts1.getResultTable());
  inc.execute();

  Commit c;
  c.addInput(inc.getResultTable());
  c.setTXContext(writeCtx);
  c.execute();

  // Now read and validate
  auto readCtx = hyrise::tx::TransactionManager::getInstance().buildContext();

  SimpleTableScan sts2;
  sts2.setTXContext(readCtx);
  sts2.addInput(t);
  sts2.setPredicate(new GreaterThanExpression<storage::hyrise_int_t>(0, 0, 0));
  sts2.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(sts2.getResultTable());
  vp.execute();

  auto result = vp.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(PosUpdateIncrementTests, increment_multiple_rows_test) {
  auto reference = io::Loader::shortcuts::load("test/reference/companies_after_mutiple_rows_increment.tbl");

  // Increment first
  auto writeCtx = hyrise::tx::TransactionManager::getInstance().buildContext();

  SimpleTableScan sts1;
  sts1.setTXContext(writeCtx);
  sts1.addInput(t);
  sts1.setPredicate(new GreaterThanExpression<storage::hyrise_int_t>(0, 0, 1));
  sts1.execute();

  PosUpdateIncrementScan inc("company_id", 23);
  inc.setTXContext(writeCtx);
  inc.addInput(sts1.getResultTable());
  inc.execute();

  Commit c;
  c.addInput(inc.getResultTable());
  c.setTXContext(writeCtx);
  c.execute();

  // Now read and validate
  auto readCtx = hyrise::tx::TransactionManager::getInstance().buildContext();

  SimpleTableScan sts2;
  sts2.setTXContext(readCtx);
  sts2.addInput(t);
  sts2.setPredicate(new GreaterThanExpression<storage::hyrise_int_t>(0, 0, 0));
  sts2.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(sts2.getResultTable());
  vp.execute();

  auto result = vp.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}
}
}
