// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IndexAwareTableScan.h"
#include "access/CreateIndex.h"
#include "helper/types.h"
#include "io/shortcuts.h"
#include "testing/test.h"
#include "helper/checked_cast.h"
#include "access/InsertScan.h"
#include "storage/Store.h"
#include "io/TransactionManager.h"

namespace hyrise {
namespace access {

class IndexAwareTableScanTests : public AccessTest {
public:
  IndexAwareTableScanTests() {}

  virtual void SetUp() {
    AccessTest::SetUp();
    t = Loader::shortcuts::load("test/index_test.tbl");
    CreateIndex ci;
    ci.addInput(t);
    ci.addField(0);
    ci.setIndexName("my_index");
    ci.execute();

    auto row = Loader::shortcuts::load("test/index_insert_test.tbl");
    storage::atable_ptr_t table(new storage::Store(row));
    auto ctx = tx::TransactionManager::getInstance().buildContext();
    InsertScan ins;
    ins.setTXContext(ctx);
    ins.addInput(t);
    ins.setInputData(row);
    ins.execute();
  }

  storage::atable_ptr_t t;
};

TEST_F(IndexAwareTableScanTests, basic_index_aware_table_scan_test) {
  // auto reference = Loader::shortcuts::load("test/reference/index_aware_test_result.tbl");

  // IndexAwareTableScan is(std::unique_ptr<EqualsExpression<hyrise_int_t> >(new EqualsExpression<hyrise_int_t>(0, 0, 200)));
  // is.addInput(t);
  // is.addField(0);
  // is.setIndexName("my_index");
  // is.setValue<hyrise_int_t>(200);
  // is.execute();

  // auto result = is.getResultTable();

  // ASSERT_TABLE_EQUAL(result, reference);
}

}
}
