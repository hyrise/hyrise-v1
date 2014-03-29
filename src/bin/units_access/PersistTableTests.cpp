// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PersistTable.h"
#include "access/RecoverTable.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class PersistTableTests : public AccessTest {};

TEST_F(PersistTableTests, basic_persist_and_restore_test) {
  const std::string tableName = "PersistTableTestTable";
  auto* sm = io::StorageManager::getInstance();
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");

  sm->loadTable(tableName, t);

  PersistTable pt;
  pt.setTableName(tableName);
  pt.execute();

  sm->removeTable(tableName);
  ASSERT_FALSE(sm->exists(tableName));

  RecoverTable rt;
  rt.setTableName(tableName);
  rt.execute();

  ASSERT_TRUE(sm->exists(tableName));
  ASSERT_TABLE_EQUAL(t, sm->getTable(tableName));
}
}
}
