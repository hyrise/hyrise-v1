// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "storage/AbstractTable.h"
#include "storage/TableFactory.h"

class TableFactoryTests : public ::hyrise::Test {};

TEST_F(TableFactoryTests, number_of_column) {
  TableFactory t;
  std::vector<const ColumnMetadata *> meta;
  meta.push_back(new ColumnMetadata("basti", IntegerType));
  hyrise::storage::atable_ptr_t  table = t.generate(&meta);
}
