// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "access/storage/GetTable.h"
#include "access/storage/SetTable.h"
#include "io/StorageManager.h"
#include "io/shortcuts.h"

namespace hyrise {
namespace access {

class GetSetTests : public AccessTest {
 protected:
  std::shared_ptr<storage::AbstractTable> _table;
  GetSetTests() : AccessTest(),
                  _table(io::Loader::shortcuts::load("test/empty.tbl")) {}
};

TEST_F(GetSetTests, basic_set) {
  SetTable st("new_table");
  st.addInput(_table);
  ASSERT_EQ(_table, st.execute()->getResultTable())
      << "Operation result should be the input table";
  ASSERT_EQ(io::StorageManager::getInstance()->getTable("new_table"), _table)
      << "Make sure table is now managed by storage manager";

}

TEST_F(GetSetTests, basic_get) {
  io::StorageManager::getInstance()->loadTable("new_table", _table);
  GetTable gt("new_table");
  ASSERT_EQ(_table, gt.execute()->getResultTable())
      << "Result table should be equal to the one loaded prior to operation";
}

TEST_F(GetSetTests, get_fails) {
  GetTable gt("non_existant");
  ASSERT_THROW({ gt.execute(); }, std::runtime_error);
}

}
}
