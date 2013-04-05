// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/UnloadAll.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class UnloadAllTests : public AccessTest {};

TEST_F(UnloadAllTests, basic_unload_all_test) {
  auto sm = StorageManager::getInstance();
  auto t1 = Loader::shortcuts::load("test/lin_xxs.tbl");
  auto t2 = Loader::shortcuts::load("test/lin_xxxs.tbl");
  auto t3 = Loader::shortcuts::load("test/10_30_group.tbl");

  ASSERT_THROW(sm->getTable("table1"), io::StorageManagerException);
  ASSERT_THROW(sm->getTable("table2"), io::StorageManagerException);
  ASSERT_THROW(sm->getTable("table3"), io::StorageManagerException);

  sm->loadTable("table1", t1);
  sm->loadTable("table2", t2);
  sm->loadTable("table3", t3);

  ASSERT_NO_THROW(sm->getTable("table1"));
  ASSERT_NO_THROW(sm->getTable("table2"));
  ASSERT_NO_THROW(sm->getTable("table3"));

  UnloadAll ua;
  ua.execute();

  ASSERT_THROW(sm->getTable("table1"), io::StorageManagerException);
  ASSERT_THROW(sm->getTable("table2"), io::StorageManagerException);
  ASSERT_THROW(sm->getTable("table3"), io::StorageManagerException);
}

}
}
