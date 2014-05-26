// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/CreateGroupkeyIndex.h"
#include "access/CreateDeltaIndex.h"
#include "access/CompoundIndexRangeScan.h"
#include "access/InsertScan.h"
#include "access/tx/Commit.h"

#include "storage/Store.h"
#include "io/shortcuts.h"
#include "io/TransactionManager.h"

#include "testing/test.h"

namespace hyrise {
namespace access {

class CompoundIndexRangeScanTests : public AccessTest {
 public:
  CompoundIndexRangeScanTests() {}

  virtual void SetUp() {
    AccessTest::SetUp();
    t = io::Loader::shortcuts::load("test/range_index_test.tbl");

    {
      CreateGroupkeyIndex ci;
      ci.addInput(t);
      ci.addField(0);
      ci.addField(2);
      ci.addField(3);
      ci.setIndexName("test_main_idx_0_2_3");
      ci.execute();

      CreateDeltaIndex cd;
      cd.addInput(t);
      cd.addField(0);
      cd.addField(2);
      cd.addField(3);
      cd.setIndexName("test_delta_idx_0_2_3");
      cd.execute();
    }

    {
      CreateGroupkeyIndex ci;
      ci.addInput(t);
      ci.addField(3);
      ci.setIndexName("test_simple_main_idx_3");
      ci.execute();

      CreateDeltaIndex cd;
      cd.addInput(t);
      cd.addField(3);
      cd.setIndexName("test_simple_delta_idx_3");
      cd.execute();
    }

    auto row = io::Loader::shortcuts::load("test/range_index_delta_test.tbl");
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

TEST_F(CompoundIndexRangeScanTests, simplePredicate) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addPredicate("col_0", (hyrise_int_t)10);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, missingDeltaIndex) {
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.addPredicate("col_0", (hyrise_int_t)10);

  ASSERT_THROW(is.execute(), std::runtime_error);
}

TEST_F(CompoundIndexRangeScanTests, missingMainIndex) {
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addPredicate("col_0", (hyrise_int_t)10);

  ASSERT_THROW(is.execute(), std::runtime_error);
}

TEST_F(CompoundIndexRangeScanTests, setMainIndexAfterPredicate) {
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addPredicate("col_0", (hyrise_int_t)10);
  ASSERT_THROW(is.setMainIndex("test_main_idx_0_2_3"), std::runtime_error);
}

TEST_F(CompoundIndexRangeScanTests, setDeltaIndexAfterPredicate) {
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.addPredicate("col_0", (hyrise_int_t)10);
  ASSERT_THROW(is.setDeltaIndex("test_delta_idx_0_2_3"), std::runtime_error);
}

TEST_F(CompoundIndexRangeScanTests, addPredicateInWrongOrder) {
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  ASSERT_THROW(is.addPredicate("col_2", (hyrise_int_t)12), std::runtime_error);
}

TEST_F(CompoundIndexRangeScanTests, addPredicateToUnindexedColumn) {
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  ASSERT_THROW(is.addPredicate("col_1", (hyrise_float_t)1.1), std::runtime_error);
}

TEST_F(CompoundIndexRangeScanTests, rangeMinPredicateValueExisting) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_2.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)60);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, rangeMinPredicateValueNotExisting) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_2.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)59);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, rangeMaxPredicateValueExisting) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_3.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)20);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, rangeMaxPredicateValueNotExisting) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_3.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)21);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, rangeMinMaxPredicateValuesExisting) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_4.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)10);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)20);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, rangeMinMaxPredicateValuesNotExisting) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_4.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)9);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)29);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, noRangeAndRangePredicatesCombined) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addPredicate("col_0", (hyrise_int_t)10);
  is.addPredicate("col_2", (hyrise_int_t)12);
  is.addRangeMinPredicate("col_3", (hyrise_int_t)11);
  is.addRangeMaxPredicate("col_3", (hyrise_int_t)14);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, firstDictValue) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_3.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)0);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)20);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, lastDictValue) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_5.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)60);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)70);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, greaterThanLastDictValue) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_2.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)60);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)72);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, minInclusiveMaxInclusive) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_6.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)10);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)60);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(CompoundIndexRangeScanTests, wholeIndexSelected) {
  auto reference = io::Loader::shortcuts::load("test/reference/range_index_test_result_7.tbl");
  auto index_ctx = tx::TransactionManager::getInstance().buildContext();

  CompoundIndexRangeScan is;
  is.addInput(t);
  is.setTXContext(index_ctx);
  is.setValidation(true);
  is.setMainIndex("test_main_idx_0_2_3");
  is.setDeltaIndex("test_delta_idx_0_2_3");
  is.addRangeMinPredicate("col_0", (hyrise_int_t)0);
  is.addRangeMaxPredicate("col_0", (hyrise_int_t)71);
  is.addRangeMinPredicate("col_2", (hyrise_int_t)2);
  is.addRangeMaxPredicate("col_2", (hyrise_int_t)73);
  is.addRangeMinPredicate("col_3", (hyrise_int_t)3);
  is.addRangeMaxPredicate("col_3", (hyrise_int_t)74);

  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

// TODO not first column delta

}  // namespace access
}  // namespace hyrise
