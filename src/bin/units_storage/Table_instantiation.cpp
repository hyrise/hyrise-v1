#include "gtest/gtest.h"

#include "helper/make_unique.h"
#include "storage/GroupMeta.h"
#include "storage/Table.h"

namespace hyrise { namespace storage {

TEST(TableInstantiation, base) {
  std::vector<ColumnGenerator> col;
  col.emplace_back(IntegerType, "col1", make_unique<OrderIndifferentDictionaryFactory>());
  col.emplace_back(IntegerType, "col2", make_unique<OrderPreservingDictionaryFactory>());
  col.emplace_back(IntegerType, "col3", make_unique<ExistingDictionary>());
  TableMetaGenerator tmg(10, make_unique<FixedLengthVectorFactory>(), std::move(col));
  TableMetaGenerator tmg(10, make_unique<BitCompressedFactory>(), std::move(col));
  Table t(std::move(tmg));
  t.print(10);
}

}}
