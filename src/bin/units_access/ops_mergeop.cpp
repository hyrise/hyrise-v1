// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "helper.h"
#include "access/MergeTable.h"
#include "access/storage/LoadFile.h"
#include "io/shortcuts.h"

namespace hyrise {
namespace access {

class MergeTableOpTests : public AccessTest {};

TEST_F(MergeTableOpTests, simple) {
  LoadFile t1("tables/employees.tbl");
  auto load_main = t1.execute()->getResultTable();

  LoadFile t2("tables/employees_new_row.tbl");
  auto load_delta = t2.execute()->getResultTable();

  MergeTable mop;
  mop.addInput(load_main);
  mop.addInput(load_delta);
  auto result = mop.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(io::Loader::shortcuts::load("test/tables/employees_revised.tbl"), result);
}

}
}

