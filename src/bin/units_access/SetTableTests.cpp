// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SetTable.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class SetTableTests : public AccessTest {};

TEST_F(SetTableTests, basic_SetTable_test) {
  auto sm = StorageManager::getInstance();
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  ASSERT_THROW(sm->getTable("myTable"), io::StorageManagerException);

  SetTable st("myTable");
  st.addInput(t);
  st.execute();

  ASSERT_TABLE_EQUAL(sm->getTable("myTable"), t);
}

}
}
