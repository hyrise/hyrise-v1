// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IndexScan.h"
#include "access/CreateIndex.h"
#include "helper/types.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class IndexScanTests : public AccessTest {
public:
  IndexScanTests() {}

  virtual void SetUp() {
    AccessTest::SetUp();
    t = io::Loader::shortcuts::load("test/index_test.tbl");
    CreateIndex ci;
    ci.addInput(t);
    ci.addField(0);
    ci.setIndexName("my_index");
    ci.execute();
  }

  storage::atable_ptr_t t;
};

TEST_F(IndexScanTests, basic_index_scan_test) {
  auto reference = io::Loader::shortcuts::load("test/reference/index_test_result.tbl");

  IndexScan is;
  is.addInput(t);
  is.addField(0);
  is.setIndexName("my_index");
  is.setValue<hyrise_int_t>(200);
  is.execute();

  auto result = is.getResultTable();

  ASSERT_TABLE_EQUAL(result, reference);
}

}
}
