// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/TableUnload.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class TableUnloadTests : public AccessTest {};

TEST_F(TableUnloadTests, basic_table_unload_test) {
  auto sm = io::StorageManager::getInstance();
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");

  ASSERT_THROW(sm->getTable("myTable"), io::ResourceManagerException);

  sm->loadTable("myTable", t);

  ASSERT_TRUE(sm->getTable("myTable")->contentEquals(t));

  TableUnload tu;
  tu.setTableName("myTable");
  tu.execute();

  ASSERT_THROW(sm->getTable("myTable"), io::ResourceManagerException);
}

}
}
