// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"

#include "io/TransactionManager.h"
#include "io/shortcuts.h"

#include "access/PosUpdateScan.h"
#include "access/SimpleTableScan.h"
#include "access/expressions/predicates.h"
#include "access/tx/Commit.h"
#include "access/tx/ValidatePositions.h"

namespace hyrise {
namespace access {

class PosUpdateScanTests : public AccessTest {
 public:
  PosUpdateScanTests() {}

  virtual void SetUp() {
    hyrise::tx::TransactionManager::getInstance().reset();
    AccessTest::SetUp();
    _companies = io::Loader::shortcuts::load("test/tables/companies.tbl");
  }

  storage::atable_ptr_t _companies;
};



TEST_F(PosUpdateScanTests, update_and_select) {

  // Do the update on microsoft
  auto writeCtx = hyrise::tx::TransactionManager::getInstance().buildContext();

  SimpleTableScan sts1;
  sts1.setTXContext(writeCtx);
  sts1.addInput(_companies);
  sts1.setPredicate(new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(0, 0, 2));
  sts1.execute();

  Json::Value update_data;
  update_data["company_id"] = 7;

  PosUpdateScan update;
  update.setRawData(update_data);
  update.setTXContext(writeCtx);
  update.addInput(sts1.getResultTable());
  update.execute();

  Commit c;
  c.addInput(update.getResultTable());
  c.setTXContext(writeCtx);
  c.execute();

  // Read values
  auto readCtx = hyrise::tx::TransactionManager::getInstance().buildContext();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(_companies);
  vp.execute();

  SimpleTableScan sts2;
  sts2.setTXContext(readCtx);
  sts2.addInput(vp.getResultTable());
  // Bug-Note: Using the GenericExpressionValue works fine
  // Bug-Note: Using the EqualsExpression returns wrong results
  // sts2.setPredicate(new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(0, 0, 7));
  sts2.setPredicate(new EqualsExpression<hyrise_int_t>(0, "company_id", 7));
  sts2.execute();
  
  auto result = sts2.getResultTable();

  // Do projection on reference
  // Bug-Note: Without update the projection with EqualsExpression works
  auto reference = io::Loader::shortcuts::load("test/reference/companies_after_one_increments.tbl");
  SimpleTableScan ref_sts;
  ref_sts.setTXContext(readCtx);
  ref_sts.addInput(reference);
  ref_sts.setPredicate(new EqualsExpression<hyrise_int_t>(0, "company_id", 7));
  ref_sts.execute();

  auto ref_result = ref_sts.getResultTable();

  ASSERT_TABLE_EQUAL(result, ref_result);
}


} // namespace access
} // namespace hyrise