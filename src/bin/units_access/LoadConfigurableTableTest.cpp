// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadConfigurableTable.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class LoadConfigurableTableTests : public AccessTest {};

TEST_F(LoadConfigurableTableTests, basic_load_configurable_table_test) {
  auto t = Loader::shortcuts::load("test/tables/employees.tbl");

  ColumnType ColumnTypes[] = {ColumnType::ColDefaultType, ColumnType::ColDefaultDictVector};

  // run test for each ColumnType
  for (ColumnType type: ColumnTypes) {

    ColumnProperties colProps;
    colProps.defaultType = type;

    LoadConfigurableTable lf("tables/employees.tbl", &colProps);
    lf.execute();

    const auto &result = lf.getResultTable();

    ASSERT_TABLE_EQUAL(result, t);
  }
}

}
}
