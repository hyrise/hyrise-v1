// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <vector>

#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "storage/GroupkeyIndex.h"
#include "storage/DeltaIndex.h"
#include "storage/Store.h"
#include "testing/test.h"
#include "storage/CompoundValueKeyBuilder.h"
#include "storage/CompoundValueIdKeyBuilder.h"
#include "access/InsertScan.h"

namespace hyrise {
namespace storage {

class CompoundGroupkeyIndexTest : public ::hyrise::Test {
  virtual void SetUp() {}
};
class CompoundDeltaIndexTest : public ::hyrise::Test {
  virtual void SetUp() {}
};

TEST_F(CompoundGroupkeyIndexTest, search_two_ints) {
  auto t = io::Loader::shortcuts::load("test/index_test.tbl");

  std::vector<field_t> fields = {0, 3};
  std::vector<hyrise_int_t> values = {30, 33};
  auto gki = std::make_shared<storage::GroupkeyIndex<compound_valueid_key_t>>(t, fields);

  CompoundValueIdKeyBuilder builder;
  size_t i = 0;
  for (auto&& field : fields) {
    auto part = t->getPart(field, 0);
    auto dict = checked_pointer_cast<storage::BaseDictionary<hyrise_int_t>>(part.table->dictionaryAt(part.column));
    builder.add(dict->getValueIdForValue(values[i]), dict->size());
    i++;
  }

  PositionRange result = gki->getPositionsForKey(builder.get());

  ASSERT_EQ(1, result.size());
  ASSERT_EQ(3, *(result.cbegin()));
}

TEST_F(CompoundGroupkeyIndexTest, search_int_and_string) {
  auto t = io::Loader::shortcuts::load("test/index_test.tbl");

  std::vector<field_t> fields = {0, 2};
  auto gki = std::make_shared<storage::GroupkeyIndex<compound_valueid_key_t>>(t, fields);

  CompoundValueIdKeyBuilder builder;
  {
    auto part = t->getPart(0, 0);
    auto dict = checked_pointer_cast<storage::BaseDictionary<hyrise_int_t>>(part.table->dictionaryAt(part.column));
    builder.add(dict->getValueIdForValue(40), dict->size());
  }
  {
    auto part = t->getPart(2, 0);
    auto dict = checked_pointer_cast<storage::BaseDictionary<hyrise_string_t>>(part.table->dictionaryAt(part.column));
    builder.add(dict->getValueIdForValue("42"), dict->size());
  }

  PositionRange result = gki->getPositionsForKey(builder.get());

  ASSERT_EQ(1, result.size());
  ASSERT_EQ(4, *(result.cbegin()));
}

TEST_F(CompoundDeltaIndexTest, search_two_ints) {
  auto t = io::Loader::shortcuts::load("test/index_test.tbl");
  auto store = checked_pointer_cast<Store>(t);
  auto i = io::Loader::shortcuts::load("test/index_insert_test.tbl");

  std::vector<field_t> fields = {0, 3};
  auto di = std::make_shared<storage::DeltaIndex<compound_value_key_t>>();
  store->addDeltaIndex(di, fields);

  access::InsertScan is;
  is.addInput(store);
  is.setInputData(i);
  is.execute();

  CompoundValueKeyBuilder builder;
  builder.add((hyrise_int_t)200);
  builder.add((hyrise_int_t)203);
  PositionRange result = di->getPositionsForKey(builder.get());

  ASSERT_EQ(1, result.size());
  ASSERT_EQ(101, *(result.cbegin()));
}
}
}
