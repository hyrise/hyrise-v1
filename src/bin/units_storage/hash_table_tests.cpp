// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <time.h>

#include "helper/stringhelpers.h"
#include "io/shortcuts.h"
#include "storage/HashTable.h"
#include "storage/Store.h"

namespace hyrise {
namespace storage {

template <typename HT>
::testing::AssertionResult TestCoverage(const atable_ptr_t &table,
                                        const field_list_t &columns) {
  HT ht(table, columns);
  if (testHashTableFullCoverage(ht, table, columns)) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() <<
           "The HashTable did not map the table correctly!";
  }
}

template <typename HT>
bool testHashTableFullCoverage(const HT &hashTable,
                               const atable_ptr_t &table,
                               const field_list_t &columns) {
  bool result = true;
  for (pos_t row = 0; row < table->size(); ++row) {
    pos_list_t positions = hashTable.get(table, columns, row);
    if (positions.empty()) {
      result = false;
      break;
    }
  }
  return result;
}

template<typename T>
class SingleHashTableTest : public ::hyrise::Test {
public:
  std::shared_ptr<AbstractTable> table;

  virtual void SetUp() {
    table = io::Loader::shortcuts::load("test/join_exchange.tbl");
  }
};

typedef ::testing::Types<SingleJoinHashTable, SingleAggregateHashTable> single_hash_types;
TYPED_TEST_CASE(SingleHashTableTest, single_hash_types);


TYPED_TEST(SingleHashTableTest, sinlge_key_test) {
  field_list_t fields {1};
  pos_t row {1};

  TypeParam htable(this->table, fields);
  auto pos_list = htable.get(this->table, fields, row);
  EXPECT_EQ(pos_list.size(), 2u);

  pos_list = htable.get(this->table, fields, {3});
  EXPECT_EQ(pos_list.size(), 3u);
}

template <typename T>
class HashTableTest : public ::hyrise::Test {
public:
  std::shared_ptr<AbstractTable> table;

  virtual void SetUp() {
    table = io::Loader::shortcuts::load("test/join_exchange.tbl");
  }
};


typedef ::testing::Types<JoinHashTable, AggregateHashTable> hash_types;
TYPED_TEST_CASE(HashTableTest, hash_types);

TYPED_TEST(HashTableTest, load) {
  field_list_t fields {1, 2};
  pos_t row {1};

  TypeParam htable(this->table, fields);
  auto pos_list = htable.get(this->table, fields, row);

  EXPECT_EQ(pos_list.size(), 2u);
  EXPECT_TRUE(contains_all(pos_list, pos_list_t {0, 1}));
}

TYPED_TEST(HashTableTest, get_pos_list) {
  field_list_t fields = {1, 2};

  TypeParam htable(this->table, fields);
  EXPECT_EQ(htable.size(), 5u);

  // For each line, we check that a line ends up in the right place
  for (size_t line = 0; line < this->table->size(); ++line) {
    auto pos_list = htable.get(this->table, fields, line);
    EXPECT_TRUE(contains_all(pos_list, pos_list_t {line}));
  }
}

TYPED_TEST(HashTableTest, load_key_test) {
  field_list_t fields = {1, 2};
  pos_t row = 1;
  TypeParam htable(this->table, fields);
  auto key = GroupKeyHash<typename TypeParam::key_t>::getGroupKey(this->table, fields, fields.size(), row);
  auto pos_list = htable.get(key);
  EXPECT_EQ(pos_list.size(), 2u);
  EXPECT_TRUE(contains_all(pos_list, pos_list_t {0, 1}));
}

const std::vector<field_list_t> combinations {{0}, {1}, {2}, {0, 1}, {1, 2},  {0, 1, 2}};

TYPED_TEST(HashTableTest, test_column_combinations) {
  hyrise::storage::atable_ptr_t table = io::Loader::shortcuts::load("test/tables/hash_table_test.tbl");
for (auto & cols: combinations) {
    SCOPED_TRACE(joinString(cols, ","));
    EXPECT_TRUE(TestCoverage<TypeParam>(table, cols));
  }
}

TYPED_TEST(HashTableTest, test_column_combinations_store) {
  hyrise::storage::atable_ptr_t store = io::Loader::shortcuts::loadMainDelta("test/tables/hash_table_test_main.tbl",
                                        "test/tables/hash_table_test_delta.tbl");
for (auto & cols: combinations) {
    SCOPED_TRACE(joinString(cols, ","));
    EXPECT_TRUE(TestCoverage<TypeParam>(store, cols));
  }
}

template <typename T>
class HashTableViewTest : public ::hyrise::Test {
protected:
  hyrise::storage::atable_ptr_t table;
  virtual void SetUp() {
    table = io::Loader::shortcuts::load("test/tables/hash_table_test.tbl");
  }
};

TYPED_TEST_CASE(HashTableViewTest, hash_types);

TYPED_TEST(HashTableViewTest, no_range) {
  field_list_t columns {0, 1, 2};

  auto hashTab = std::make_shared<TypeParam>(this->table, columns);
  auto view = hashTab->view(0, 0);

  EXPECT_TRUE(view->size() == 0);
  EXPECT_TRUE(view->getMapBegin() == view->getMapEnd());
  EXPECT_TRUE(view->getMapEnd() == hashTab->getMapBegin());

  auto itMap = hashTab->getMapBegin();

  for (; itMap != hashTab->getMapEnd(); ++itMap) {
    auto key = itMap->first;
    EXPECT_TRUE(view->get(key).empty());
  }
}

TYPED_TEST(HashTableViewTest, full_range) {
  field_list_t columns {0, 1, 2};
  auto table = this->table;

  auto hashTab = std::make_shared<TypeParam>(table, columns);
  auto view = hashTab->view(0, table->size());

  EXPECT_TRUE(view->size() == table->size());

  for (auto itMap = hashTab->getMapBegin();
       itMap != hashTab->getMapEnd();
       ++itMap) {
    auto key = itMap->first;
    EXPECT_TRUE(!view->get(key).empty());
  }
}

TYPED_TEST(HashTableViewTest, partial_range) {
  field_list_t columns {0, 1, 2};
  auto table = this->table;
  const size_t start_row = 1, end_row = table->size() - 1;

  auto hashTab = std::make_shared<TypeParam>(table, columns);
  auto view = hashTab->view(start_row, end_row);

  EXPECT_EQ(view->size(), end_row - start_row);
  auto hashPosition = hashTab->getMapBegin();
  auto hashEnd = hashTab->getMapBegin();

  std::advance(hashPosition, start_row); // advance hashPosition to row 1
  std::advance(hashEnd, end_row);

  for (;
       hashPosition != hashEnd;
       std::advance(hashPosition, 1)) {
    auto key = hashPosition->first;
    EXPECT_TRUE(!view->get(key).empty());
  }
}

} } // namespace hyrise::storage

