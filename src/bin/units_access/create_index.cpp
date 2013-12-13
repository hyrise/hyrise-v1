// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/CreateIndex.h"

#include "testing/test.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "storage/AbstractIndex.h"
#include "storage/InvertedIndex.h"

namespace hyrise {
namespace access {

class IndexTests : public AccessTest {};

TEST_F(IndexTests, basic_index_test) {
  std::string table = "test_table1";

  auto t = io::Loader::shortcuts::load("test/index_test.tbl");

  hyrise::access::CreateIndex i;
  i.addInput(t);
  i.addField(0);
  i.setIndexName(table);
  i.execute();

  auto sm = io::StorageManager::getInstance();
  auto index = std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_int_t>> (sm->getInvertedIndex(table));
  int key = 30;
  pos_list_t positions = index->getPositionsForKey(key);
  pos_t first_pos = positions[0];
  pos_t expected = 3;

  ASSERT_EQ(first_pos, expected);
}

TEST_F(IndexTests, multiple_positions_index_test) {
  std::string table = "test_table2";
  auto t = io::Loader::shortcuts::load("test/index_test2.tbl");
  CreateIndex i;
  i.addInput(t);
  i.addField(1);
  i.setIndexName(table);
  i.execute();

  auto sm = io::StorageManager::getInstance();
  auto index = std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_string_t>> (sm->getInvertedIndex(table));
  std::string key1 = "Bayer";
  std::string key2 = "RWE";
  pos_list_t positions1 = index->getPositionsForKey(key1);
  pos_list_t positions2 = index->getPositionsForKey(key2);
  pos_list_t expected1;
  expected1.push_back(2);
  expected1.push_back(4);
  expected1.push_back(10);
  pos_list_t expected2;
  expected2.push_back(6);
  expected2.push_back(7);
  expected2.push_back(12);

  ASSERT_EQ(positions1, expected1);
  ASSERT_EQ(positions2, expected2);
}

TEST_F(IndexTests, basic_index_test_float) {
  std::string table = "test_table3";

  auto t = io::Loader::shortcuts::load("test/index_test.tbl");

  hyrise::access::CreateIndex i;
  i.addInput(t);
  i.addField(1);
  i.setIndexName(table);
  i.execute();

  auto sm = io::StorageManager::getInstance();
  auto index = std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_float_t>> (sm->getInvertedIndex(table));
  float key = 71.1;
  pos_list_t positions = index->getPositionsForKey(key);
  pos_t first_pos = positions[0];
  pos_t expected = 7;

  ASSERT_EQ(first_pos, expected);
}

}
}

