// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "storage/AbstractTable.h"
#include "storage/TableFactory.h"

namespace hyrise {
namespace storage {

class TableFactoryTests : public Test {};

TEST_F(TableFactoryTests, number_of_column) {
  TableFactory t;
  std::vector<ColumnMetadata > meta;
  meta.emplace_back("basti", IntegerType);
  hyrise::storage::atable_ptr_t  table = t.generate(&meta);
}

} } // namespace hyrise::storage
