// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>

#include "helper.h"

#include <storage/PointerCalculator.h>
#include <storage/PointerCalculatorFactory.h>
#include <io/StorageManager.h>
#include <io/shortcuts.h>
#include <io/loaders.h>

class PointerCalcTests : public ::hyrise::StorageManagerTest {};

TEST_F(PointerCalcTests, init_pc) {
  {
    hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");
    ASSERT_TRUE(t->columnCount() == 10);



    //PointerCalculator *pc = new PointerCalculator(t);
    //ASSERT_EQ((size_t) 10, pc->columnCount());
    //delete pc;
  }
}



TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count) {
  StorageManager *sm = StorageManager::getInstance();
  std::string table_description = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 2_R | 3_R | 4_R | 5_R | 6_R";

  Loader::params params;
  params.setHeader(StringHeader(table_description));
  params.setInput(CSVInput("tables/10_col_only_data.tbl", CSVInput::params().setUnsafe(true).setCSVParams(csv::HYRISE_FORMAT)));

  sm->loadTable("mytab", params);

  hyrise::storage::atable_ptr_t  t = sm->getTable("mytab");



  ASSERT_EQ((unsigned) 7, t->partitionCount());


  std::vector<field_t> *tmp_fd = new std::vector<field_t>;
  tmp_fd->push_back(1);
  tmp_fd->push_back(2);
  tmp_fd->push_back(3);
  tmp_fd->push_back(4);

  hyrise::storage::atable_ptr_t  res = PointerCalculatorFactory::createPointerCalculatorNonRef(t, tmp_fd, nullptr);
  ASSERT_EQ((unsigned) 4, res->partitionCount());
}

TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count_4_groups) {
  StorageManager *sm = StorageManager::getInstance();
  std::string table_description = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 1_R | 1_R | 2_R | 3_R | 4_R";


  Loader::params params;
  params.setHeader(StringHeader(table_description));
  params.setInput(CSVInput("tables/10_col_only_data.tbl", CSVInput::params().setUnsafe(true).setCSVParams(csv::HYRISE_FORMAT)));
  sm->loadTable("mytab", params);

  hyrise::storage::atable_ptr_t  t = sm->getTable("mytab");
  ASSERT_EQ((unsigned) 5, t->partitionCount());


  std::vector<field_t> *tmp_fd = new std::vector<field_t>;
  tmp_fd->push_back(1);
  tmp_fd->push_back(2);
  tmp_fd->push_back(3);
  tmp_fd->push_back(4);

  hyrise::storage::atable_ptr_t  res = PointerCalculatorFactory::createPointerCalculatorNonRef(t, tmp_fd, nullptr);
  ASSERT_EQ((unsigned) 2, res->partitionCount());
}


TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count_1_group) {
  StorageManager *sm = StorageManager::getInstance();
  std::string table_description = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 1_R | 1_R | 2_R | 3_R | 4_R";

  Loader::params params;
  params.setHeader(StringHeader(table_description));
  params.setInput(CSVInput("tables/10_col_only_data.tbl", CSVInput::params().setUnsafe(true).setCSVParams(csv::HYRISE_FORMAT)));

  sm->loadTable("mytab", params);

  hyrise::storage::atable_ptr_t  t = sm->getTable("mytab");
  ASSERT_EQ((unsigned) 5, t->partitionCount());
  ASSERT_EQ(7u, t->columnCount());


  std::vector<field_t> *tmp_fd = new std::vector<field_t>;
  tmp_fd->push_back(1);
  tmp_fd->push_back(2);
  tmp_fd->push_back(3);

  hyrise::storage::atable_ptr_t  res = PointerCalculatorFactory::createPointerCalculatorNonRef(t, tmp_fd, nullptr);
  ASSERT_EQ(1u, res->partitionCount());

  ASSERT_EQ(3u, res->partitionWidth(0));
}

TEST_F(PointerCalcTests, pc_from_vertical_table_slice_count_1_group_projection) {
  StorageManager *sm = StorageManager::getInstance();
  std::string table_description = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 1_R | 1_R | 2_R | 3_R | 4_R";

  Loader::params params;
  params.setHeader(StringHeader(table_description));
  params.setInput(CSVInput("tables/10_col_only_data.tbl", CSVInput::params().setUnsafe(true).setCSVParams(csv::HYRISE_FORMAT)));

  sm->loadTable("mytab", params);

  hyrise::storage::atable_ptr_t  t = sm->getTable("mytab");
  ASSERT_EQ((unsigned) 5, t->partitionCount());


  std::vector<field_t> *tmp_fd = new std::vector<field_t>;
  tmp_fd->push_back(1);

  hyrise::storage::atable_ptr_t  res = PointerCalculatorFactory::createPointerCalculatorNonRef(t, tmp_fd, nullptr);
  ASSERT_EQ(1u, res->partitionCount());
  ASSERT_EQ(1u, res->partitionWidth(0));

}


TEST_F(PointerCalcTests, pc_using_factory) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");
  auto pc = PointerCalculatorFactory::createPointerCalculatorNonRef(t);

  ASSERT_TRUE(pc->columnCount() == 10);
  size_t tmp = 0;
  ValueId i = pc->getValueId(tmp, tmp);
  ASSERT_EQ((value_id_t) 0, i.valueId);
  ASSERT_EQ(0, i.table);

}

TEST_F(PointerCalcTests, pc_in_vector) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");
  auto pc = PointerCalculatorFactory::createPointerCalculatorNonRef(t);

  ASSERT_TRUE(pc->columnCount() == 10);
  size_t tmp = 0;
  ValueId i = pc->getValueId(tmp, tmp);
  ASSERT_EQ((value_id_t) 0, i.valueId);
  ASSERT_EQ(i.table, 0);


  std::vector<std::shared_ptr<PointerCalculator>> ll;
  ll.push_back(pc);

  i = ll[0]->getValueId(tmp, tmp);
  ASSERT_EQ((value_id_t) 0, i.valueId);
  ASSERT_EQ(i.table, 0);

}

TEST_F(PointerCalcTests, pc_on_selected_columns) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");

  field_list_t field_definition;
  field_definition.push_back(1); // col_1
  field_definition.push_back(3); // col_3
  field_definition.push_back(5); // col_5
  field_definition.push_back(7); // col_7

  auto pc = PointerCalculatorFactory::createPointerCalculatorNonRef(t, new field_list_t(field_definition), nullptr);

  ASSERT_TRUE(pc->columnCount() == 4);

  ASSERT_TRUE(pc->metadataAt(0)->matches(t->metadataAt(1)));
  ASSERT_TRUE(pc->metadataAt(1)->matches(t->metadataAt(3)));
  ASSERT_TRUE(pc->metadataAt(2)->matches(t->metadataAt(5)));
  ASSERT_TRUE(pc->metadataAt(3)->matches(t->metadataAt(7)));

}

