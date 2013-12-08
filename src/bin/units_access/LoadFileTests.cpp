// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/LoadFile.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class LoadFileTests : public AccessTest {};

TEST_F(LoadFileTests, basic_load_file_test) {
  auto t = io::Loader::shortcuts::load("test/tables/employees.tbl");

  LoadFile lf("tables/employees.tbl");
  lf.execute();

  const auto &result = lf.getResultTable();

  ASSERT_TABLE_EQUAL(result, t);
}

}
}
