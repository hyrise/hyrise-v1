// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/GetTable.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class GetTableTests : public AccessTest {
public:
  GetTableTests() : _table(io::Loader::shortcuts::load("test/empty.tbl")) {
  }

  storage::atable_ptr_t _table;
};

TEST_F(GetTableTests, basic_get_table_test) {
  io::StorageManager::getInstance()->loadTable("new_table", _table);

  GetTable gt("new_table");
  gt.execute();

  const auto &result = gt.getResultTable();

  ASSERT_TABLE_EQUAL(_table, result);
}

TEST_F(GetTableTests, get_table_fail_test) {
  GetTable gt("non_existent");
  ASSERT_THROW(gt.execute(), std::runtime_error);
}

}
}
