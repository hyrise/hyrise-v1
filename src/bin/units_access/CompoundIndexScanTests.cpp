// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/CompoundIndexScan.h"
#include "access/CreateGroupkeyIndex.h"
#include "access/CreateDeltaIndex.h"
#include <access.h>
#include "helper/types.h"
#include "io/shortcuts.h"
#include "testing/test.h"
#include "helper/checked_cast.h"
#include "access/InsertScan.h"
#include "storage/Store.h"
#include "io/TransactionManager.h"
#include "access/expressions/pred_LessThanExpression.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class CompoundIndexScanTests : public AccessTest {
 public:
  CompoundIndexScanTests() {}

  virtual void SetUp() {
    AccessTest::SetUp();
    t = io::Loader::shortcuts::load("test/index_test.tbl");

    {
      CreateGroupkeyIndex ci;
      ci.addInput(t);
      ci.addField(0);
      ci.addField(3);
      ci.setIndexName("test_main_idx_0_and_3");
      ci.execute();

      CreateDeltaIndex cd;
      cd.addInput(t);
      cd.addField(0);
      cd.addField(3);
      cd.setIndexName("test_delta_idx_0_and_3");
      cd.execute();
    }

    {
      CreateGroupkeyIndex ci;
      ci.addInput(t);
      ci.addField(0);
      ci.addField(2);
      ci.setIndexName("test_main_idx_0_and_2");
      ci.execute();

      CreateDeltaIndex cd;
      cd.addInput(t);
      cd.addField(0);
      cd.addField(2);
      cd.setIndexName("test_delta_idx_0_and_2");
      cd.execute();
    }

    auto row = io::Loader::shortcuts::load("test/index_insert_test.tbl");
    auto ctx = tx::TransactionManager::getInstance().buildContext();
    InsertScan ins;
    ins.setTXContext(ctx);
    ins.addInput(t);
    ins.setInputData(row);
    ins.execute();

    Commit c;
    c.setTXContext(ctx);
    c.execute();
  }

  storage::atable_ptr_t t;
};

TEST_F(CompoundIndexScanTests, two_ints) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result.tbl");

  CompoundIndexScan is;
  is.addInput(t);
  is.setMainIndex("test_main_idx_0_and_3");
  is.setDeltaIndex("test_delta_idx_0_and_3");
  is.addPredicate(0, (hyrise_int_t)200);
  is.addPredicate(3, (hyrise_int_t)203);
  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}



TEST_F(CompoundIndexScanTests, two_ints_validation_nonunique) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();
  CompoundIndexScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_and_3");
  is.setDeltaIndex("test_delta_idx_0_and_3");
  is.addPredicate(0, (hyrise_int_t)200);
  is.addPredicate(3, (hyrise_int_t)203);
  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}


TEST_F(CompoundIndexScanTests, two_ints_validation_nonunique_partial_key) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();
  CompoundIndexScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_and_3");
  is.setDeltaIndex("test_delta_idx_0_and_3");
  is.addPredicate(0, (hyrise_int_t)200);
  // is.addPredicate(3, (hyrise_int_t)203);
  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}



TEST_F(CompoundIndexScanTests, two_ints_validation_unique) {

  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result_unique.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexScan is;
  is.addInput(t);

  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setUniqueIndex(true);  // allow index scan to get out once the first valid entry is found.

  is.setMainIndex("test_main_idx_0_and_3");
  is.setDeltaIndex("test_delta_idx_0_and_3");
  // 123 _ _ 123 is one hit in delta
  is.addPredicate(0, (hyrise_int_t)123);
  is.addPredicate(3, (hyrise_int_t)123);
  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}



TEST_F(CompoundIndexScanTests, int_and_a_string) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result.tbl");

  CompoundIndexScan is;
  is.addInput(t);
  is.setMainIndex("test_main_idx_0_and_2");
  is.setDeltaIndex("test_delta_idx_0_and_2");
  is.addPredicate(0, (hyrise_int_t)200);
  is.addPredicate(2, (hyrise_string_t)("202"));
  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexScanTests, wrong_column) {
  CompoundIndexScan is;
  is.addInput(t);
  is.setMainIndex("test_main_idx_0_and_3");
  ASSERT_THROW(is.addPredicate(4, (hyrise_int_t)203), std::runtime_error);
}

TEST_F(CompoundIndexScanTests, wrong_order) {
  CompoundIndexScan is;
  is.addInput(t);
  is.setMainIndex("test_main_idx_0_and_3");
  ASSERT_THROW(is.addPredicate(3, (hyrise_int_t)203), std::runtime_error);
}

TEST_F(CompoundIndexScanTests, not_all_columns) {
  CompoundIndexScan is;
  is.addInput(t);
  is.setMainIndex("test_main_idx_0_and_3");
  is.addPredicate(0, (hyrise_int_t)200);
  ASSERT_THROW(is.execute(), std::runtime_error);
}

}  // namespace access
}  // namespace hyrise
