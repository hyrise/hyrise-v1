// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadConfigurableTable.h"

#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

ColumnType ColumnTypes[] = {ColumnType::ColDefaultType, ColumnType::ColDefaultDictVector};

class LoadConfigurableTableTests : public AccessTest {};

TEST_F(LoadConfigurableTableTests, basic_load_configurable_table_test) {

  auto t = Loader::shortcuts::load("test/tables/employees.tbl");

  // run test for each ColumnType
  for (ColumnType type: ColumnTypes) {

    std::shared_ptr<ColumnProperties> colProps (new ColumnProperties);
    colProps->setDefaultType(type);

    LoadConfigurableTable lct("tables/employees.tbl", colProps);
    lct.execute();

    const auto &result = lct.getResultTable();

    ASSERT_TABLE_EQUAL(result, t);
  }
}

TEST_F(LoadConfigurableTableTests, load_configurable_table_test_different_types) {

  auto t = Loader::shortcuts::load("test/tables/employees.tbl");

  std::shared_ptr<ColumnProperties> colProps(new ColumnProperties);
  
  int typeIndex = 0;
  int maxType = sizeof(ColumnTypes)/sizeof(ColumnTypes[0]) - 1;
  for (size_t c = 0; c < t->columnCount(); ++c)
  {
    colProps->setType(c, ColumnTypes[typeIndex]);
    if (++typeIndex > maxType)
      typeIndex = 0;
  }
  LoadConfigurableTable lct("tables/employees.tbl", colProps);
  lct.execute();

  const auto &result = lct.getResultTable();

  ASSERT_TABLE_EQUAL(result, t);
}

}
}
