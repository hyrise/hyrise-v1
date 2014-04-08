// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "helper.h"

#include "io/shortcuts.h"

#include "storage/AbstractTable.h"
#include "storage/DictionaryFactory.h"
#include "storage/FixedLengthVector.h"
#include "storage/OrderIndifferentDictionary.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/RawTable.h"
#include "storage/SimpleStore.h"
#include "storage/storage_types.h"
#include "storage/Store.h"

namespace hyrise {
namespace storage {

class TableTests : public Test {

 public:
  metadata_list intList(size_t num = 2) {
    metadata_list result;
    for (size_t i = 0; i < num; ++i)
      result.push_back(ColumnMetadata::metadataFromString("INTEGER", "col" + std::to_string(i)));
    return result;
  }

  metadata_list intstringlist() {
    metadata_list result;
    result.push_back(ColumnMetadata::metadataFromString("INTEGER", "col1"));
    result.push_back(ColumnMetadata::metadataFromString("STRING", "col1"));
    return result;
  }
};

TEST_F(TableTests, builds_correct_meta_data_for_no_dict) {
  auto t = ColumnMetadata::metadataFromString("INTEGER_NO_DICT", "col1");
  ASSERT_EQ(IntegerNoDictType, t.getType());
}


TEST_F(TableTests, does_copy_structure_copy_structure) {
  hyrise::storage::atable_ptr_t input = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  ASSERT_EQ(3u, input->partitionCount());

  hyrise::storage::atable_ptr_t copy = input->copy_structure();
  ASSERT_EQ(3u, input->partitionCount()) << "Copied table should have the same number of containers";
}

TEST_F(TableTests, copy_structure_replacement) {
  auto input =
      io::Loader::shortcuts::load("test/lin_xxs.tbl", io::Loader::params().setReturnsMutableVerticalTable(true));
  ASSERT_EQ(3u, input->partitionCount());
  auto order_indifferent = [](DataType dt) { return makeDictionary(types::getUnorderedType(dt)); };
  auto order_preserving = [](DataType dt) { return makeDictionary(types::getOrderedType(dt)); };

  auto b = [](std::size_t cols) { return std::make_shared<FixedLengthVector<value_id_t>>(cols, 0); };
  hyrise::storage::atable_ptr_t copy = input->copy_structure(order_preserving, b);
  ASSERT_TRUE(std::dynamic_pointer_cast<OrderPreservingDictionary<hyrise_int_t>>(copy->dictionaryAt(0, 0)) != nullptr);
  hyrise::storage::atable_ptr_t copy2 = input->copy_structure(order_indifferent, b);
  ASSERT_TRUE(std::dynamic_pointer_cast<ConcurrentUnorderedDictionary<hyrise_int_t>>(copy2->dictionaryAt(0, 0)) !=
              nullptr);
}

TEST_F(TableTests, number_of_column) {
  hyrise::storage::atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  ASSERT_TRUE(t->numberOfColumn("col_0") == 0);
  ASSERT_TRUE(t->numberOfColumn("col_2") == 2);
  // ASSERT_TRUE( t->numberOfColumn("does_not_exist") == -1 );
}

TEST_F(TableTests, bit_compression_test) {
  hyrise::storage::atable_ptr_t main = io::Loader::shortcuts::load("test/bittest.tbl");

  ASSERT_TRUE(main->getValue<hyrise_int_t>(0, 0) == 4);
  ASSERT_TRUE(main->getValue<hyrise_int_t>(0, 1) == 0);
  ASSERT_TRUE(main->getValue<hyrise_int_t>(0, 2) == 2);

  ASSERT_TRUE(main->getValue<float>(1, 0) == (float)1.1);
  ASSERT_TRUE(main->getValue<float>(1, 1) == (float)3.1);
  ASSERT_TRUE(main->getValue<float>(1, 2) == (float)2.1);

  ASSERT_TRUE(main->getValue<std::string>(2, 0) == "doppelt");
  ASSERT_TRUE(main->getValue<std::string>(2, 1) == "doppelt");
  ASSERT_TRUE(main->getValue<std::string>(2, 2) == "acht");

  size_t zero = 0, one = 1, two = 2;

  ASSERT_TRUE(main->getValueId(zero, zero).valueId == 2);
  ASSERT_TRUE(main->getValueId(zero, one).valueId == 0);
  ASSERT_TRUE(main->getValueId(zero, two).valueId == 1);

  ASSERT_TRUE(main->getValueId(one, zero).valueId == 0);
  ASSERT_TRUE(main->getValueId(one, one).valueId == 2);
  ASSERT_TRUE(main->getValueId(one, two).valueId == 1);

  ASSERT_TRUE(main->getValueId(two, zero).valueId == 1);
  ASSERT_TRUE(main->getValueId(two, one).valueId == 1);
  ASSERT_TRUE(main->getValueId(two, two).valueId == 0);
}

TEST_F(TableTests, test_modifiable_table) {
  auto a = empty_table(10, 2);
  a->setValue<hyrise_int_t>(0, 0, 100);
  a->setValue<hyrise_int_t>(0, 1, 200);
  ASSERT_EQ(a->getValue<hyrise_int_t>(0, 0), 100);
  ASSERT_EQ(a->getValue<hyrise_int_t>(0, 1), 200);
}


TEST_F(TableTests, test_table_copy) {
  auto a = empty_table(10, 2);
  a->setValue<hyrise_int_t>(0, 0, 100);
  a->setValue<hyrise_int_t>(0, 1, 200);

  hyrise::storage::atable_ptr_t b = a->copy();
  b->setValue<hyrise_int_t>(0, 0, 50);
  b->setValue<hyrise_int_t>(0, 1, 100);
}

TEST_F(TableTests, test_main_storage_is_fixedLengthVector) {
  hyrise::storage::atable_ptr_t input = io::Loader::shortcuts::load("test/tables/companies.tbl");
  const auto& store = std::dynamic_pointer_cast<Store>(input);
  const auto& main = store->getMainTable();
  const auto& avs = main->getAttributeVectors(0);
  for (const auto& av : avs) {
    ASSERT_NE(std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(av.attribute_vector), nullptr);
  }
}
}
}
