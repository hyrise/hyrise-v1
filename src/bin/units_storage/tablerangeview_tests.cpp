// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>

#include "helper.h"

#include <storage/TableRangeView.h>
#include <storage/TableRangeViewFactory.h>
#include <io/StorageManager.h>
#include <io/shortcuts.h>
#include <io/loaders.h>

class TableRangeViewTests : public ::hyrise::StorageManagerTest {};

TEST_F(TableRangeViewTests, init_pc) {
  {
    hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");

    TableRangeView *trv = new TableRangeView(t, 0, t->size());
    ASSERT_EQ(t->size(), trv->size());

    size_t row = 2;
    size_t column = 2;

    ValueId i = trv->getValueId(column, row);
    ValueId i2 = t->getValueId(column,row);
    ASSERT_EQ(i.valueId, i2.valueId);

    delete trv;
  }
}

TEST_F(TableRangeViewTests, pc_using_factory) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxs.tbl");
  size_t start = 5;
  size_t end = 20;
  size_t row = 10;
  size_t column = 1;

  auto trv = TableRangeViewFactory::createView(t, start, end);

  ASSERT_EQ(trv->size(), (end-start));

  ValueId i = trv->getValueId(column, row-start);
  ValueId i2 = t->getValueId(column,row);

  ASSERT_EQ(i.valueId, i2.valueId);
}
