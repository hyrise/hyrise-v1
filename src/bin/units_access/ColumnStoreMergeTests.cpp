// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "access/CreateGroupkeyIndex.h"
#include "access/MergeTable.h"
#include "access/InsertScan.h"

#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "io/TransactionManager.h"

#include "storage/GroupkeyIndex.h"
#include "storage/Store.h"
#include "storage/TableGenerator.h"

namespace hyrise {
namespace access {

class MergeColumnStoreTests : public AccessTest {};

void insertDelta(std::shared_ptr<storage::AbstractTable> main, std::string deltaPath) {
  auto delta = io::Loader::shortcuts::load(deltaPath);

  auto ctx = tx::TransactionManager::getInstance().buildContext();

  InsertScan is;
  is.setTXContext(ctx);
  is.addInput(main);
  is.setInputData(delta);
  is.execute();
}

template <std::size_t numberOfOffsets, std::size_t numberOfPostings, typename T>
void assertIndexEQ(std::string indexName, std::array<pos_t, numberOfOffsets>& offsetsResults, std::array<pos_t, numberOfPostings>& postingsResults) {
  auto sm = io::StorageManager::getInstance();

  auto index = std::dynamic_pointer_cast<storage::GroupkeyIndex<T>>(sm->getInvertedIndex(indexName));

  auto postingsIterator = index->postingsBegin();
  auto offsetsIterator = index->offsetsBegin();

  for (uint32_t i = 0; i < numberOfOffsets; ++i, ++offsetsIterator) {
    ASSERT_EQ(*offsetsIterator, offsetsResults[i]);
  }
  ASSERT_EQ(offsetsIterator, index->offsetsEnd());

  for (uint32_t i = 0; i < numberOfPostings; ++i, ++postingsIterator) {
    ASSERT_EQ(*postingsIterator, postingsResults[i]);
  }
  ASSERT_EQ(postingsIterator, index->postingsEnd());
}

TEST_F(MergeColumnStoreTests, singleColumnInt) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_singleColumnInt.tbl");

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto &result = mcs.getResultTable();

  ASSERT_EQ(13u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnFloat) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_value.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_singleColumnFloat.tbl");

  insertDelta(table, "test/tables/employee_value_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto &result = mcs.getResultTable();

  ASSERT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnString) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_name.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_singleColumnString.tbl");

  insertDelta(table, "test/tables/employee_name_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto &result = mcs.getResultTable();

  ASSERT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, threeColumns) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_threeColumns.tbl");

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto &result = mcs.getResultTable();

  ASSERT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnDoubleMerge) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_singleColumnDoubleMerge.tbl");

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  insertDelta(table, "test/tables/employee_id_delta2.tbl");

  mcs.addInput(table);
  mcs.execute();

  const auto &result = mcs.getResultTable();

  ASSERT_EQ(20u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnWithGroupKeyIndex) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_main_idx_0");
  ci.execute();

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  std::array<pos_t, 10> offsetsResults = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13};
  std::array<pos_t, 13> postingsResults = {0, 1, 11, 3, 2, 4, 5, 6, 10, 8, 9, 12, 7};
  assertIndexEQ<10, 13, hyrise_int_t>("test_main_idx_0", offsetsResults, postingsResults);
}

TEST_F(MergeColumnStoreTests, threeColumnsWithGroupKeyIndex) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(1);
  ci.setIndexName("test_main_idx_1");
  ci.execute();

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  std::array<pos_t, 8> offsetsResults = {0, 1, 2, 3, 5, 6, 7, 9};
  std::array<pos_t, 9> postingsResults = {1, 4, 2, 7, 8, 3, 5, 0, 6};
  assertIndexEQ<8, 9, hyrise_float_t>("test_main_idx_1", offsetsResults, postingsResults);
}

TEST_F(MergeColumnStoreTests, threeColumnsWithThreeGroupKeyIndexes) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_main_idx_0");
  ci.execute();

  CreateGroupkeyIndex ci2;
  ci2.addInput(table);
  ci2.addField(1);
  ci2.setIndexName("test_main_idx_1");
  ci2.execute();

  CreateGroupkeyIndex ci3;
  ci3.addInput(table);
  ci3.addField(2);
  ci3.setIndexName("test_main_idx_2");
  ci3.execute();

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  std::array<pos_t, 10> offsetsResults0 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::array<pos_t, 9> postingsResults0 = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  assertIndexEQ<10, 9, hyrise_int_t>("test_main_idx_0", offsetsResults0, postingsResults0);

  std::array<pos_t, 8> offsetsResults1 = {0, 1, 2, 3, 5, 6, 7, 9};
  std::array<pos_t, 9> postingsResults1 = {1, 4, 2, 7, 8, 3, 5, 0, 6};
  assertIndexEQ<8, 9, hyrise_float_t>("test_main_idx_1", offsetsResults1, postingsResults1);

  std::array<pos_t, 10> offsetsResults2 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::array<pos_t, 9> postingsResults2 = {2, 8, 5, 4, 7, 1, 0, 6, 3};
  assertIndexEQ<10, 9, hyrise_string_t>("test_main_idx_2", offsetsResults2, postingsResults2);
}

TEST_F(MergeColumnStoreTests, singleColumnWithGroupKeyIndexDoubleMerge) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_main_idx_0");
  ci.execute();

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  insertDelta(table, "test/tables/employee_id_delta2.tbl");

  mcs.addInput(table);
  mcs.execute();

  std::array<pos_t, 11> offsetsResults = {0, 1, 4, 5, 7, 8, 11, 13, 17, 19, 20};
  std::array<pos_t, 20> postingsResults = {0, 1, 11, 17, 3, 2, 4, 5, 6, 10, 16, 8, 14, 9, 12, 15, 18, 7, 13, 19};
  assertIndexEQ<11, 20, hyrise_int_t>("test_main_idx_0", offsetsResults, postingsResults);
}

}
}
