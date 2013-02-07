#include "testing/test.h"
#include "io/shortcuts.h"
#include "storage/AbstractTable.h"

TEST(AbstractTable_getAttributeVectors, test) {
  const auto& table = Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->getAttributeVectors(1);
  EXPECT_EQ(2u, vectors.size());
}
