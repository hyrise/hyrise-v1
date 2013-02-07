#include "testing/test.h"
#include "memory/MemalignStrategy.h"
#include "storage/AbstractTable.h"
#include "storage/TableFactory.h"


class TableFactoryTests : public ::hyrise::Test {};

TEST_F(TableFactoryTests, number_of_column) {
  TableFactory<MemalignStrategy<64> > t;
  std::vector<const ColumnMetadata *> meta;
  meta.push_back(new ColumnMetadata("basti", IntegerType));
  AbstractTable::SharedTablePtr  table = t.generate(&meta);
}
