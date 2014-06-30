// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "io/shortcuts.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

TEST(AbstractTable_getAttributeVectors, test) {
  const auto& table = io::Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->getAttributeVectors(1);
  EXPECT_EQ(2u, vectors.size());
}

TEST(AbstractTable_getAttributeVectors, test2) {
  const auto& table = io::Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->allParts();
  std::cout << std::endl;
  for (auto& p : vectors) {
    std::cout << p.column << " " << p.row << " " << p.table->size() << std::endl;
  }
  std::cout << std::endl;
  // EXPECT_EQ(2u, vectors.size());
}
}
}  // namespace hyrise::storage
