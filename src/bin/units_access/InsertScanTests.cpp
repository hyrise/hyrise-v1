// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/InsertScan.h"
#include "io/shortcuts.h"
#include "io/TransactionManager.h"
#include "storage/Store.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class InsertScanTests : public AccessTest {};

TEST_F(InsertScanTests, basic_insert_scan_test) {
  auto row = Loader::shortcuts::load("test/insert_one.tbl");
  storage::atable_ptr_t table(new storage::Store(row));

  auto ctx = tx::TransactionManager::getInstance().buildContext();

  InsertScan is;
  is.setTXContext(ctx);
  is.addInput(table);
  is.setInputData(row);
  is.execute();

  const auto &result = std::dynamic_pointer_cast<const storage::Store>(is.getResultTable());

  ASSERT_TABLE_EQUAL(result->getDeltaTable(), row);
}

}
}
