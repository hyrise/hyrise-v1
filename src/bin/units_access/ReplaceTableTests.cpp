// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/ReplaceTable.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class ReplaceTableTests : public AccessTest {};

TEST_F(ReplaceTableTests, basic_replace_table_test) {
  auto t1 = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto t2 = io::Loader::shortcuts::load("test/10_30_group.tbl");

  auto sm = io::StorageManager::getInstance();
  sm->loadTable("replaceMe", t1);

  ASSERT_TABLE_EQUAL(t1, sm->getTable("replaceMe"));

  ReplaceTable rt("replaceMe");
  rt.addInput(t2);
  rt.execute();

  ASSERT_TABLE_EQUAL(t2, sm->getTable("replaceMe"));
}

}
}
