// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IndexAwareTableScan.h"
#include "access/CreateGroupkeyIndex.h"
#include "access/CreateDeltaIndex.h"
#include <access.h>
#include "helper/types.h"
#include "io/shortcuts.h"
#include "testing/test.h"
#include "helper/checked_cast.h"
#include "access/InsertScan.h"
#include "storage/Store.h"
#include "io/TransactionManager.h"
#include "access/expressions/pred_LessThanExpression.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class IndexAwareTableScanTests : public AccessTest {
 public:
  IndexAwareTableScanTests() {}

  virtual void SetUp() {
    AccessTest::SetUp();
    t = io::Loader::shortcuts::load("test/index_test.tbl");

    CreateGroupkeyIndex ci;
    ci.addInput(t);
    ci.addField(0);
    ci.setIndexName("mcidx__foo__main__col_0");
    ci.execute();

    CreateDeltaIndex cd;
    cd.addInput(t);
    cd.addField(0);
    cd.setIndexName("mcidx__foo__delta__col_0");
    cd.execute();

    CreateGroupkeyIndex ci2;
    ci2.addInput(t);
    ci2.addField(1);
    ci2.setIndexName("mcidx__foo__main__col_1");
    ci2.execute();

    CreateDeltaIndex cd2;
    cd2.addInput(t);
    cd2.addField(1);
    cd2.setIndexName("mcidx__foo__delta__col_1");
    cd2.execute();

    auto row = io::Loader::shortcuts::load("test/index_insert_test.tbl");
    auto ctx = tx::TransactionManager::getInstance().buildContext();
    InsertScan ins;
    ins.setTXContext(ctx);
    ins.addInput(t);
    ins.setInputData(row);
    ins.execute();
  }

  storage::atable_ptr_t t;
};

TEST_F(IndexAwareTableScanTests, basic_index_aware_table_scan_test_eq) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result.tbl");

  IndexAwareTableScan is;
  is.addInput(t);
  is.setTableName("foo");
  is.addField("col_0");
  is.setPredicate(new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(0, "col_0", 200));
  is.execute();
  auto result = is.getResultTable();
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(IndexAwareTableScanTests, basic_index_aware_table_scan_test_lt) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result_lt.tbl");

  IndexAwareTableScan is;
  is.addInput(t);
  is.setTableName("foo");
  is.addField("col_0");
  is.setPredicate(new GenericExpressionValue<hyrise_int_t, std::less<hyrise_int_t>>(0, "col_0", 200));
  is.execute();
  auto result = is.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(IndexAwareTableScanTests, basic_index_aware_table_scan_test_intersect) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_aware_test_result_intersect.tbl");

  IndexAwareTableScan is;
  is.addInput(t);
  is.setTableName("foo");
  is.addField("col_0");
  CompoundExpression* ce = new CompoundExpression(AND);
  ce->add(new GenericExpressionValue<hyrise_int_t, std::greater<hyrise_int_t>>(0, "col_0", 110));
  ce->add(new GenericExpressionValue<hyrise_int_t, std::less<hyrise_int_t>>(0, "col_1", 200));
  is.setPredicate(ce);
  is.execute();
  auto result = is.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

}  // namespace access
}  // namespace hyrise
