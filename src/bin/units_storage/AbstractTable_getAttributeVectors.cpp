// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <list>
#include <stack>

#include "testing/test.h"
#include "io/shortcuts.h"
#include "storage/AbstractTable.h"

#include "storage/Table.h"
#include "storage/MutableVerticalTable.h"

#include "storage/column_extract.h"

namespace hyrise {
namespace storage {

class TableVisitor : public StorageVisitor {
 public:
  Visitation visit(const Table& t) {
    std::cout << &t << std::endl;
    return Visitation::next;  // continue visitation
  }
};


TEST(AbstractTable_getAttributeVectors, test) {
  const auto& table = io::Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->getAttributeVectors(1);

  EXPECT_EQ(2u, vectors.size());

  auto parts = column_parts_extract(*table.get(), 1, 0, table->size());
  EXPECT_EQ(2u, parts.size());
}

TEST(AbstractTable_getAttributeVectors, test2) {
  const auto& table = io::Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->allParts();
  EXPECT_EQ(2u, vectors.size());
}

TEST(AbstractTable_getAttributeVectors, test3) {
  const auto& table = io::Loader::shortcuts::load("test/partitioned_test.tbl");
  TableVisitor dv;
  table->accept(dv);
}
}
}  // namespace hyrise::storage
