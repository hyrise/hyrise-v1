// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/GetLastRecords.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class GetLastRecordsTests : public AccessTest {};

TEST_F(GetLastRecordsTests, basic_get_last_records_test) {
  auto t = io::Loader::shortcuts::load("test/tables/companies.tbl");
  auto reference = io::Loader::shortcuts::load("test/tables/companies_last2.tbl");

  GetLastRecords gs;
  gs.addInput(t);
  gs._records = 2;
  gs.execute();
  const auto& result = gs.getResultTable();

  ASSERT_TABLE_EQUAL(reference, result);
}


TEST_F(GetLastRecordsTests, get_last_records_more_records_test) {
  auto t = io::Loader::shortcuts::load("test/tables/companies.tbl");

  GetLastRecords gs;
  gs.addInput(t);
  gs._records = 5;
  gs.execute();
  const auto& result = gs.getResultTable();

  ASSERT_TABLE_EQUAL(t, result);
}

TEST_F(GetLastRecordsTests, get_last_records_empty_table_test) {
  auto t = io::Loader::shortcuts::load("test/empty.tbl");

  GetLastRecords gs;
  gs.addInput(t);
  gs._records = 2;
  gs.execute();
  const auto& result = gs.getResultTable();

  ASSERT_TABLE_EQUAL(t, result);
}
}
}
