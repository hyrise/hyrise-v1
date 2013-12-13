// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/UnloadAll.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class UnloadAllTests : public AccessTest {};

TEST_F(UnloadAllTests, basic_unload_all_test) {
  auto sm = io::StorageManager::getInstance();
  auto t1 = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto t2 = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  auto t3 = io::Loader::shortcuts::load("test/10_30_group.tbl");

  ASSERT_THROW(sm->getTable("table1"), io::ResourceManagerException);
  ASSERT_THROW(sm->getTable("table2"), io::ResourceManagerException);
  ASSERT_THROW(sm->getTable("table3"), io::ResourceManagerException);

  sm->loadTable("table1", t1);
  sm->loadTable("table2", t2);
  sm->loadTable("table3", t3);

  ASSERT_NO_THROW(sm->getTable("table1"));
  ASSERT_NO_THROW(sm->getTable("table2"));
  ASSERT_NO_THROW(sm->getTable("table3"));

  UnloadAll ua;
  ua.execute();

  ASSERT_THROW(sm->getTable("table1"), io::ResourceManagerException);
  ASSERT_THROW(sm->getTable("table2"), io::ResourceManagerException);
  ASSERT_THROW(sm->getTable("table3"), io::ResourceManagerException);
}

}
}
