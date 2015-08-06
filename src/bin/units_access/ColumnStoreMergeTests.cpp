// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "access/CreateGroupkeyIndex.h"
#include "access/MergeTable.h"
#include "access/InsertScan.h"
#include "access/CreateIndex.h"
#include "access/CreateDeltaIndex.h"

#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "io/TransactionManager.h"
#include "io/ResourceManager.h"

#include "storage/GroupkeyIndex.h"
#include "storage/InvertedIndex.h"
#include "storage/Store.h"

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
void expectGKIndexEQ(std::string indexName,
                     std::array<pos_t, numberOfOffsets>& offsetsResults,
                     std::array<pos_t, numberOfPostings>& postingsResults) {
  auto sm = io::StorageManager::getInstance();

  auto index = std::dynamic_pointer_cast<storage::GroupkeyIndex<T>>(sm->getInvertedIndex(indexName));

  auto postingsIterator = index->postingsBegin();
  auto offsetsIterator = index->offsetsBegin();

  for (uint32_t i = 0; i < numberOfOffsets; ++i, ++offsetsIterator) {
    EXPECT_EQ(*offsetsIterator, offsetsResults[i]);
  }
  EXPECT_EQ(offsetsIterator, index->offsetsEnd());

  for (uint32_t i = 0; i < numberOfPostings; ++i, ++postingsIterator) {
    EXPECT_EQ(*postingsIterator, postingsResults[i]);
  }
  EXPECT_EQ(postingsIterator, index->postingsEnd());
}

TEST_F(MergeColumnStoreTests, rowStoreThrows) {
  auto table = io::Loader::shortcuts::load("test/tables/employees.tbl");

  insertDelta(table, "test/tables/employees.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);

  EXPECT_THROW(mcs.execute(), std::runtime_error);
}

TEST_F(MergeColumnStoreTests, singleColumnInt) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/ColumnStoreMerge_singleColumnInt.tbl");

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(13u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnFloat) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_value.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/ColumnStoreMerge_singleColumnFloat.tbl");

  insertDelta(table, "test/tables/employee_value_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnString) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_name.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/ColumnStoreMerge_singleColumnString.tbl");

  insertDelta(table, "test/tables/employee_name_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, emptyDelta) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  insertDelta(table, "test/tables/employee_id_empty.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(7u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, emptyMain) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id_empty.tbl");
  auto reference = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  insertDelta(table, "test/tables/employee_id.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(7u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, emptyMainDelta) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id_empty.tbl");
  auto reference = io::Loader::shortcuts::load("test/tables/employee_id_empty.tbl");

  insertDelta(table, "test/tables/employee_id_empty.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(0, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, threeColumns) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/ColumnStoreMerge_threeColumns.tbl");

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, singleColumnDoubleMerge) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/ColumnStoreMerge_singleColumnDoubleMerge.tbl");

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  insertDelta(table, "test/tables/employee_id_delta2.tbl");

  mcs.addInput(table);
  mcs.execute();

  const auto& result = mcs.getResultTable();

  EXPECT_EQ(20u, result->size());
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
  expectGKIndexEQ<10, 13, hyrise_int_t>("test_main_idx_0", offsetsResults, postingsResults);
}

TEST_F(MergeColumnStoreTests, singleColumnWithGroupKeyIndexForceFullIndexRebuild) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_main_idx_0");
  ci.execute();

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  std::array<pos_t, 10> offsetsResults = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13};
  std::array<pos_t, 13> postingsResults = {0, 1, 11, 3, 2, 4, 5, 6, 10, 8, 9, 12, 7};
  expectGKIndexEQ<10, 13, hyrise_int_t>("test_main_idx_0", offsetsResults, postingsResults);
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
  expectGKIndexEQ<8, 9, hyrise_float_t>("test_main_idx_1", offsetsResults, postingsResults);
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
  expectGKIndexEQ<10, 9, hyrise_int_t>("test_main_idx_0", offsetsResults0, postingsResults0);

  std::array<pos_t, 8> offsetsResults1 = {0, 1, 2, 3, 5, 6, 7, 9};
  std::array<pos_t, 9> postingsResults1 = {1, 4, 2, 7, 8, 3, 5, 0, 6};
  expectGKIndexEQ<8, 9, hyrise_float_t>("test_main_idx_1", offsetsResults1, postingsResults1);

  std::array<pos_t, 10> offsetsResults2 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::array<pos_t, 9> postingsResults2 = {2, 8, 5, 4, 7, 1, 0, 6, 3};
  expectGKIndexEQ<10, 9, hyrise_string_t>("test_main_idx_2", offsetsResults2, postingsResults2);
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
  expectGKIndexEQ<11, 20, hyrise_int_t>("test_main_idx_0", offsetsResults, postingsResults);
}

TEST_F(MergeColumnStoreTests, threeColumnsWithOneInvertedIndex) {
  pos_list_t expected1;
  expected1.push_back(2);
  expected1.push_back(4);
  expected1.push_back(10);
  pos_list_t expected2;
  expected2.push_back(6);
  expected2.push_back(7);
  expected2.push_back(12);

  pos_list_t expectedLT = {0, 1};
  pos_list_t expectedLTE = {0, 1, 2, 4, 10};
  pos_list_t expectedBetween = {2, 3, 4, 5, 6, 7, 9, 10, 12};
  pos_list_t expectedGT = {3, 5, 6, 7, 8, 9, 11, 12};
  pos_list_t expectedGTE = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  std::string key1 = "Bayer";
  std::string key2 = "RWE";

  auto sm = io::StorageManager::getInstance();

  auto table = io::Loader::shortcuts::load("test/tables/index_test2_main.tbl");

  CreateIndex i;
  i.addInput(table);
  i.addField(1);
  i.setIndexName("test_main_idx_0");
  i.execute();

  insertDelta(table, "test/tables/index_test2_delta.tbl");

  auto index =
      std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_string_t>>(sm->getInvertedIndex("test_main_idx_0"));
  pos_list_t positions1 = index->getPositionsForKey(key1);
  pos_list_t positions2 = index->getPositionsForKey(key2);

  pos_list_t positionsLT = index->getPositionsForKeyLT(key1);
  pos_list_t positionsLTE = index->getPositionsForKeyLTE(key1);
  pos_list_t positionsBetween = index->getPositionsForKeyBetween(key1, key2);
  pos_list_t positionsGT = index->getPositionsForKeyGT(key1);
  pos_list_t positionsGTE = index->getPositionsForKeyGTE(key1);


  EXPECT_NE(positions1, expected1);
  EXPECT_NE(positions2, expected2);
  EXPECT_EQ(positionsLT, expectedLT);
  EXPECT_NE(positionsLTE, expectedLTE);
  EXPECT_NE(positionsBetween, expectedBetween);
  EXPECT_NE(positionsGT, expectedGT);
  EXPECT_NE(positionsGTE, expectedGTE);

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  // the merge creates the full table and the assertions turn true
  index = std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_string_t>>(sm->getInvertedIndex("test_main_idx_0"));
  positions1 = index->getPositionsForKey(key1);
  positions2 = index->getPositionsForKey(key2);

  positionsLT = index->getPositionsForKeyLT(key1);
  positionsLTE = index->getPositionsForKeyLTE(key1);
  positionsBetween = index->getPositionsForKeyBetween(key1, key2);
  positionsGT = index->getPositionsForKeyGT(key1);
  positionsGTE = index->getPositionsForKeyGTE(key1);

  EXPECT_EQ(positions1, expected1);
  EXPECT_EQ(positions2, expected2);
  EXPECT_EQ(positionsLT, expectedLT);
  EXPECT_EQ(positionsLTE, expectedLTE);
  EXPECT_EQ(positionsBetween, expectedBetween);
  EXPECT_EQ(positionsGT, expectedGT);
  EXPECT_EQ(positionsGTE, expectedGTE);
}

TEST_F(MergeColumnStoreTests, threeColumnsWithGroupKeyAndInvertedIndex) {
  pos_list_t expected1;
  expected1.push_back(2);
  expected1.push_back(4);
  expected1.push_back(10);
  pos_list_t expected2;
  expected2.push_back(6);
  expected2.push_back(7);
  expected2.push_back(12);

  pos_list_t expectedLT = {0, 1};
  pos_list_t expectedLTE = {0, 1, 2, 4, 10};
  pos_list_t expectedBetween = {2, 3, 4, 5, 6, 7, 9, 10, 12};
  pos_list_t expectedGT = {3, 5, 6, 7, 8, 9, 11, 12};
  pos_list_t expectedGTE = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  std::string key1 = "Bayer";
  std::string key2 = "RWE";

  auto sm = io::StorageManager::getInstance();

  auto table = io::Loader::shortcuts::load("test/tables/index_test2_main.tbl");

  CreateIndex i;
  i.addInput(table);
  i.addField(1);
  i.setIndexName("test_main_idx_0");
  i.execute();

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(2);
  ci.setIndexName("test_main_idx_1");
  ci.execute();

  insertDelta(table, "test/tables/index_test2_delta.tbl");

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  auto invertedIndex =
      std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_string_t>>(sm->getInvertedIndex("test_main_idx_0"));
  pos_list_t positions1 = invertedIndex->getPositionsForKey(key1);
  pos_list_t positions2 = invertedIndex->getPositionsForKey(key2);

  pos_list_t positionsLT = invertedIndex->getPositionsForKeyLT(key1);
  pos_list_t positionsLTE = invertedIndex->getPositionsForKeyLTE(key1);
  pos_list_t positionsBetween = invertedIndex->getPositionsForKeyBetween(key1, key2);
  pos_list_t positionsGT = invertedIndex->getPositionsForKeyGT(key1);
  pos_list_t positionsGTE = invertedIndex->getPositionsForKeyGTE(key1);

  EXPECT_EQ(positions1, expected1);
  EXPECT_EQ(positions2, expected2);
  EXPECT_EQ(positionsLT, expectedLT);
  EXPECT_EQ(positionsLTE, expectedLTE);
  EXPECT_EQ(positionsBetween, expectedBetween);
  EXPECT_EQ(positionsGT, expectedGT);
  EXPECT_EQ(positionsGTE, expectedGTE);

  auto groupkeyIndex =
      std::dynamic_pointer_cast<storage::GroupkeyIndex<hyrise_int_t>>(sm->getInvertedIndex("test_main_idx_1"));

  std::array<pos_t, 7> offsetsResults = {0, 1, 4, 7, 10, 12, 13};
  std::array<pos_t, 13> postingsResults = {3, 5, 6, 9, 0, 4, 11, 2, 7, 10, 1, 8, 12};
  expectGKIndexEQ<7, 13, hyrise_int_t>("test_main_idx_1", offsetsResults, postingsResults);
}

TEST_F(MergeColumnStoreTests, deltaIndicesGetRemoved) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  CreateDeltaIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_delta_idx_0");
  ci.execute();

  CreateDeltaIndex ci2;
  ci2.addInput(table);
  ci2.addField(1);
  ci2.setIndexName("test_delta_idx_1");
  ci2.execute();

  MergeColumnStore mcs;
  mcs.addInput(table);
  mcs.execute();

  io::StorageManager* sm = io::StorageManager::getInstance();
  EXPECT_THROW(sm->getInvertedIndex("test_delta_idx_0"), hyrise::io::ResourceNotExistsException);
  EXPECT_THROW(sm->getInvertedIndex("test_delta_idx_1"), hyrise::io::ResourceNotExistsException);
}

TEST_F(MergeColumnStoreTests, sortedMergeInt) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/two_columns.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_two_columns.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_two_columns.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(0);
  cid.addField(1);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, sortedMergeIntFirstColumnIndifferent) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/two_columns_first_column_indifferent.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_two_columns_first_column_indifferent.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_two_columns_first_column_indifferent.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(0);
  cid.addField(1);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, sortedMergeString) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/string.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_string.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_string.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(0);
  cid.addField(1);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, sortedMergeDoubleAndEmptyMain) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/two_columns.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/empty_two_columns.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_two_columns.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(0);
  cid.addField(1);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  access::CreateDeltaIndex cid2;
  cid2.addInput(store);
  cid2.addField(0);
  cid2.addField(1);
  cid2.setIndexName("test_delta_idx_0");
  cid2.execute();

  delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_two_columns.tbl");

  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, sortedMergeEmptyDelta) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/two_columns.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/reference/two_columns.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/empty_two_columns.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(0);
  cid.addField(1);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, sortedMergeFourColumns) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/four_columns.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_four_columns.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_four_columns.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(1);
  cid.addField(2);
  cid.addField(0);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("test_delta_idx_0");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(MergeColumnStoreTests, sortedMergeThrowsOnWrongIndexName) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/four_columns.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_four_columns.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_four_columns.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(1);
  cid.addField(2);
  cid.addField(0);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("PeterSippelIndex");
  mcs.setForceFullIndexRebuild(true);
  EXPECT_THROW(mcs.execute(), std::runtime_error);
}

TEST_F(MergeColumnStoreTests, unSortedMergeInt) {
  auto reference = io::Loader::shortcuts::load("test/sortedMerge/reference/two_columns_unsorted.tbl");
  auto t = io::Loader::shortcuts::load("test/sortedMerge/tables/sorted_main_two_columns.tbl");
  auto store = checked_pointer_cast<storage::Store>(t);

  auto delta = io::Loader::shortcuts::load("test/sortedMerge/tables/delta_two_columns.tbl");

  access::CreateDeltaIndex cid;
  cid.addInput(store);
  cid.addField(0);
  cid.addField(1);
  cid.setIndexName("test_delta_idx_0");
  cid.execute();

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(delta);
  is.execute();

  access::MergeColumnStore mcs;
  mcs.addInput(store);
  mcs.setSortIndexName("");
  mcs.setForceFullIndexRebuild(true);
  mcs.execute();

  auto result = mcs.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

}
}
