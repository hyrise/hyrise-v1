// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadConfigurableTable.h"
#include <string>

#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

std::string tableFileName("test/tables/employees.tbl");
ColumnType ColumnTypes[] = {ColumnType::ColDefaultType, ColumnType::ColDefaultDictVector};

class LoadConfigurableTableTests : public AccessTest {};

TEST_F(LoadConfigurableTableTests, basic_load_configurable_table_test) {

  auto t = Loader::shortcuts::load("test/tables/employees.tbl");

  // run test for each ColumnType
  for (ColumnType type: ColumnTypes) {

    ColumnProperties colProps;
    colProps.defaultType = type;

    LoadConfigurableTable lct("tables/employees.tbl", &colProps);
    lct.execute();

    const auto &result = lct.getResultTable();

    ASSERT_TABLE_EQUAL(result, t);
  }
}

TEST_F(LoadConfigurableTableTests, load_configurable_table_test_different_types) {

  auto t = Loader::shortcuts::load("test/tables/employees.tbl");

  ColumnProperties colProps;
  
  int typeIndex = 0;
  int maxType = sizeof(ColumnTypes)/sizeof(ColumnTypes[0]) - 1;
  for (size_t c = 0; c < t->columnCount(); ++c)
  {
    colProps.setType(c, ColumnTypes[typeIndex++]);
    if (typeIndex >= maxType)
      typeIndex = 0;
  }
  LoadConfigurableTable lct("tables/employees.tbl", &colProps);
  lct.execute();

  const auto &result = lct.getResultTable();

  ASSERT_TABLE_EQUAL(result, t);
}

}
}
