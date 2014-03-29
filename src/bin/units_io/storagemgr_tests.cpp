// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <io/shortcuts.h>
#include <io/StorageManager.h>
#include <storage/MutableVerticalTable.h>
#include "storage/Store.h"
#include "helper/checked_cast.h"

namespace hyrise {
namespace io {

class StorageManagerTests : public ::hyrise::Test {

 public:
  StorageManagerTests() { sm = StorageManager::getInstance(); }

  virtual void SetUp() { sm->removeAll(); }

  StorageManager* sm;
};

TEST_F(StorageManagerTests, base) {
  StorageManager* sm2;
  sm2 = StorageManager::getInstance();

  ASSERT_TRUE(sm == sm2);
}

TEST_F(StorageManagerTests, load_table_multiple_contexts) {
  {
    StorageManager* sm = StorageManager::getInstance();
    sm->loadTableFile("LINXXS", "lin_xxs.tbl");
  }

  {
    StorageManager* sm = StorageManager::getInstance();
    ASSERT_TRUE((bool)sm->getTable("LINXXS"));
    hyrise::storage::atable_ptr_t t = sm->getTable("LINXXS");
  }

  {
    StorageManager* sm = StorageManager::getInstance();
    sm->removeTable("LINXXS");
  }
  StorageManager* sm = StorageManager::getInstance();

  ASSERT_EQ(0u, sm->getTableNames().size());

  sm->removeAll();
}

TEST_F(StorageManagerTests, load_table) {
  sm->loadTableFile("LINXXS", "lin_xxs.tbl");

  hyrise::storage::atable_ptr_t tbl = sm->getTable("LINXXS");
  hyrise::storage::atable_ptr_t ref = Loader::shortcuts::load("test/lin_xxs.tbl");
  ASSERT_TRUE(tbl->contentEquals(ref));
  sm->removeTable("LINXXS");
  ASSERT_EQ(0u, sm->getTableNames().size());
}

TEST_F(StorageManagerTests, load_table_header_data) {
  sm->loadTableFileWithHeader("HEADERDATA", "header/data.tbl", "header/header.data");

  hyrise::storage::atable_ptr_t tbl = sm->getTable("HEADERDATA");
  hyrise::storage::atable_ptr_t ref =
      Loader::shortcuts::loadWithHeader("test/header/data.tbl", "test/header/header.data");

  ASSERT_TRUE(tbl->contentEquals(ref));

  sm->removeTable("HEADERDATA");

  ASSERT_EQ(0u, sm->getTableNames().size());
}

TEST_F(StorageManagerTests, load_persist_and_recover_table) {

  sm->loadTableFile("LINXXS", "lin_xxs.tbl");
  ASSERT_TRUE(sm->exists("LINXXS"));
  sm->persistTable("LINXXS");
  sm->removeTable("LINXXS");
  ASSERT_FALSE(sm->exists("LINXXS"));
  sm->recoverTable("LINXXS");
  ASSERT_TRUE(sm->exists("LINXXS"));
}
}
}  // namespace hyrise::io
