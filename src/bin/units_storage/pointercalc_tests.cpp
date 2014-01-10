// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>

#include "helper.h"

#include <storage/PointerCalculator.h>
#include <io/StorageManager.h>
#include <io/StringLoader.h>
#include <io/shortcuts.h>
#include <io/loaders.h>

namespace hyrise {
namespace storage {

class PointerCalcTests : public StorageManagerTest {
 protected:
  std::string table_7 = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 2_R | 3_R | 4_R | 5_R | 6_R";
  std::string table_5 = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 1_R | 1_R | 2_R | 3_R | 4_R";
};

atable_ptr_t createTable(std::string description) {
  io::Loader::params params;
  params.setHeader(io::StringHeader(description));
  params.setInput(io::CSVInput("test/tables/10_col_only_data.tbl", io::CSVInput::params().setUnsafe(true).setCSVParams(io::csv::HYRISE_FORMAT)));
  return io::Loader::load(params);
}

TEST_F(PointerCalcTests, rename_field) {
  auto t = createTable(table_7);
  auto res = PointerCalculator::create(t);
  ASSERT_EQ(t->nameOfColumn(0), res->nameOfColumn(0));
  res->rename(0, "NoMobile");
  ASSERT_EQ("NoMobile", res->nameOfColumn(0));
}

TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count) {
  auto t = createTable(table_7);
  ASSERT_EQ(7u, t->partitionCount());
  auto tmp_fd = new std::vector<field_t> {1, 2, 3, 4};
  auto res = PointerCalculator::create(t, nullptr, tmp_fd);
  ASSERT_EQ(4u, res->partitionCount());
}

TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count_4_groups) {
  auto t = createTable(table_5);
  ASSERT_EQ(5u, t->partitionCount());
  auto tmp_fd = new std::vector<field_t> {1, 2, 3, 4};
  auto res = PointerCalculator::create(t, nullptr, tmp_fd);
  ASSERT_EQ(2u, res->partitionCount());
}


TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count_1_group) {
  auto t = createTable(table_5);
  ASSERT_EQ(5u, t->partitionCount());
  ASSERT_EQ(7u, t->columnCount());
  auto tmp_fd = new std::vector<field_t> {1, 2, 3};
  auto res = PointerCalculator::create(t, nullptr, tmp_fd);
  ASSERT_EQ(1u, res->partitionCount());
  ASSERT_EQ(3u, res->partitionWidth(0));
}

TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count_1_group_projection) {
  auto t = createTable(table_5);
  ASSERT_EQ(5u, t->partitionCount());
  ASSERT_EQ(7u, t->columnCount());
  auto tmp_fd = new std::vector<field_t> {1};
  auto res = PointerCalculator::create(t, nullptr, tmp_fd);
  ASSERT_EQ(1u, res->partitionCount());
  ASSERT_EQ(1u, res->partitionWidth(0));
}


TEST_F(PointerCalcTests, pc_using_factory) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto pc = PointerCalculator::create(t);

  ASSERT_TRUE(pc->columnCount() == 10);
  size_t tmp = 0;
  ValueId i = pc->getValueId(tmp, tmp);
  ASSERT_EQ((value_id_t) 0, i.valueId);
  ASSERT_EQ(0, i.table);

}


TEST_F(PointerCalcTests, pc_on_selected_columns) {
  atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");

  field_list_t field_definition {1, 3, 5, 7};

  auto pc = PointerCalculator::create(t, nullptr, new field_list_t(field_definition));

  ASSERT_TRUE(pc->columnCount() == 4);

  ASSERT_TRUE(pc->metadataAt(0).matches(t->metadataAt(1)));
  ASSERT_TRUE(pc->metadataAt(1).matches(t->metadataAt(3)));
  ASSERT_TRUE(pc->metadataAt(2).matches(t->metadataAt(5)));
  ASSERT_TRUE(pc->metadataAt(3).matches(t->metadataAt(7)));
}

} } // namespace hyrise::storage

