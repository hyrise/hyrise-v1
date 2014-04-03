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

class StoreAltMergeTests : public AccessTest {};

void insertDelta(std::shared_ptr<storage::AbstractTable> main, std::string deltaPath) {
  auto delta = io::Loader::shortcuts::load(deltaPath);

  auto ctx = tx::TransactionManager::getInstance().buildContext();

  InsertScan is;
  is.setTXContext(ctx);
  is.addInput(main);
  is.setInputData(delta);
  is.execute();
}

TEST_F(StoreAltMergeTests, singleColumn) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_singleColumn.tbl");

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeStoreAlt msa;
  msa.addInput(table);
  msa.execute();

  const auto &result = msa.getResultTable();

  ASSERT_EQ(13u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(StoreAltMergeTests, threeColumns) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_threeColumns.tbl");

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  MergeStoreAlt msa;
  msa.addInput(table);
  msa.execute();

  const auto &result = msa.getResultTable();

  ASSERT_EQ(9u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(StoreAltMergeTests, singleColumnDoubleMerge) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/AltStoreMerge_singleColumnDoubleMerge.tbl");

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeStoreAlt msa;
  msa.addInput(table);
  msa.execute();

  insertDelta(table, "test/tables/employee_id_delta2.tbl");

  msa.addInput(table);
  msa.execute();

  const auto &result = msa.getResultTable();

  ASSERT_EQ(20u, result->size());
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(StoreAltMergeTests, singleColumnWithGroupKeyIndex) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  const uint32_t numberOfOffsets = 10;
  const uint32_t numberOfPostings = 13;

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_main_idx_0");
  ci.execute();

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeStoreAlt msa;
  msa.addInput(table);
  msa.execute();

  auto sm = io::StorageManager::getInstance();
  auto index = std::dynamic_pointer_cast<storage::GroupkeyIndex<hyrise_int_t>>(sm->getInvertedIndex("test_main_idx_0"));

  auto postingsIterator = index->postingsBegin();
  auto offsetsIterator = index->offsetsBegin();

  pos_t offsetsResults[numberOfOffsets] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13};
  pos_t postingsResults[numberOfPostings] = {0, 1, 11, 3, 2, 4, 5, 6, 10, 8, 9, 12, 7};

  for (uint32_t i = 0; i < numberOfOffsets; ++i, ++offsetsIterator) {
    ASSERT_EQ(*offsetsIterator, offsetsResults[i]);
  }
  ASSERT_EQ(offsetsIterator, index->offsetsEnd());

  for (uint32_t i = 0; i < numberOfPostings; ++i, ++postingsIterator) {
    ASSERT_EQ(*postingsIterator, postingsResults[i]);
  }
  ASSERT_EQ(postingsIterator, index->postingsEnd());
}

TEST_F(StoreAltMergeTests, threeColumnsWithGroupKeyIndex) {
  auto table = io::Loader::shortcuts::load("test/tables/employeesAlternative.tbl");

  const uint32_t numberOfOffsets = 8;
  const uint32_t numberOfPostings = 9;

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(1);
  ci.setIndexName("test_main_idx_1");
  ci.execute();

  insertDelta(table, "test/tables/employeesAlternative_delta.tbl");

  MergeStoreAlt msa;
  msa.addInput(table);
  msa.execute();

  auto sm = io::StorageManager::getInstance();
  auto index = std::dynamic_pointer_cast<storage::GroupkeyIndex<hyrise_float_t>>(sm->getInvertedIndex("test_main_idx_1"));

  auto postingsIterator = index->postingsBegin();
  auto offsetsIterator = index->offsetsBegin();

  pos_t offsetsResults[numberOfOffsets] = {0, 1, 2, 3, 5, 6, 7, 9};
  pos_t postingsResults[numberOfPostings] = {1, 4, 2, 7, 8, 3, 5, 0, 6};

  for (uint32_t i = 0; i < numberOfOffsets; ++i, ++offsetsIterator) {
    ASSERT_EQ(*offsetsIterator, offsetsResults[i]);
  }
  ASSERT_EQ(offsetsIterator, index->offsetsEnd());

  for (uint32_t i = 0; i < numberOfPostings; ++i, ++postingsIterator) {
    ASSERT_EQ(*postingsIterator, postingsResults[i]);
  }
  ASSERT_EQ(postingsIterator, index->postingsEnd());
}

TEST_F(StoreAltMergeTests, singleColumnWithGroupKeyIndexDoubleMerge) {
  auto table = io::Loader::shortcuts::load("test/tables/employee_id.tbl");

  const uint32_t numberOfOffsets = 11;
  const uint32_t numberOfPostings = 20;

  CreateGroupkeyIndex ci;
  ci.addInput(table);
  ci.addField(0);
  ci.setIndexName("test_main_idx_0");
  ci.execute();

  insertDelta(table, "test/tables/employee_id_delta.tbl");

  MergeStoreAlt msa;
  msa.addInput(table);
  msa.execute();

  insertDelta(table, "test/tables/employee_id_delta2.tbl");

  msa.addInput(table);
  msa.execute();

  auto sm = io::StorageManager::getInstance();
  auto index = std::dynamic_pointer_cast<storage::GroupkeyIndex<hyrise_int_t>>(sm->getInvertedIndex("test_main_idx_0"));

  auto postingsIterator = index->postingsBegin();
  auto offsetsIterator = index->offsetsBegin();

  pos_t offsetsResults[numberOfOffsets] = {0, 1, 4, 5, 7, 8, 11, 13, 17, 19, 20};
  pos_t postingsResults[numberOfPostings] = {0, 1, 11, 17, 3, 2, 4, 5, 6, 10, 16, 8, 14, 9, 12, 15, 18, 7, 13, 19};

  for (uint32_t i = 0; i < numberOfOffsets; ++i, ++offsetsIterator) {
    ASSERT_EQ(*offsetsIterator, offsetsResults[i]);
  }
  ASSERT_EQ(offsetsIterator, index->offsetsEnd());

  for (uint32_t i = 0; i < numberOfPostings; ++i, ++postingsIterator) {
    ASSERT_EQ(*postingsIterator, postingsResults[i]);
  }
  ASSERT_EQ(postingsIterator, index->postingsEnd());
}

}
}
