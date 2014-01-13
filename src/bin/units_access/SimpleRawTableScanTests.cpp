// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleRawTableScan.h"
#include "access/expressions/predicates.h"
#include "io/shortcuts.h"
#include "storage/RawTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class SimpleRawTableScanTests : public AccessTest {
public:
  storage::atable_ptr_t createRawTable() {
    storage::metadata_vec_t columns({storage::ColumnMetadata::metadataFromString("INTEGER", "col1"),
                                     storage::ColumnMetadata::metadataFromString("STRING", "col2"),
                                     storage::ColumnMetadata::metadataFromString("FLOAT", "col3") });
    auto main = std::make_shared<storage::RawTable>(columns);
    storage::rawtable::RowHelper rh(columns);
    unsigned char *data = nullptr;

    for(size_t i=0; i<10; i++) {
      rh.set<storage::hyrise_int_t>(0, i);
      rh.set<storage::hyrise_string_t>(1, "SomeText" + std::to_string(i));
      rh.set<storage::hyrise_float_t>(2, 1.1*i);
      data = rh.build();
      main->appendRow(data);
      free(data);
      rh.reset();
    }

    return main;
  }
};

TEST_F(SimpleRawTableScanTests, basic_simple_raw_table_scan_test) {
  auto t = createRawTable();
  auto expr = new EqualsExpressionRaw<storage::hyrise_int_t>(t, 0, 5);

  SimpleRawTableScan srts(expr);
  srts.addInput(t);
  srts.execute();

  const auto &result = srts.getResultTable();

  ASSERT_EQ(1u, result->size());
  ASSERT_EQ(5u, result->getValue<storage::hyrise_int_t>(0, 0));
}

TEST_F(SimpleRawTableScanTests, simple_raw_table_scan_fail_on_not_raw_test) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto expr = new EqualsExpressionRaw<storage::hyrise_int_t>(t, 0, 5);

  SimpleRawTableScan srts(expr);
  srts.addInput(t);

  ASSERT_THROW(srts.execute(), std::runtime_error);
}

}
}
