// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadWithDefaultDict.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class LoadWithDefaultDictTests : public AccessTest {};

TEST_F(LoadWithDefaultDictTests, basic_load_with_default_dict_test) {
  auto t = Loader::shortcuts::load("test/tables/employees.tbl");

  LoadWithDefaultDict lDD("tables/employees.tbl");
  lDD.execute();

  const auto &result = lDD.getResultTable();

  ASSERT_TABLE_EQUAL(result, t);
}

}
}
