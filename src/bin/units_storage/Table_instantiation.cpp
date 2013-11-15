#include "gtest/gtest.h"

#include "helper/make_unique.h"

#include "storage/OrderIndifferentDictionary.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/FixedLengthVector.h"
#include "storage/TableDefinition.h"
#include "storage/Table.h"

namespace hyrise { namespace storage {

TEST(TableInstantiation, different_dict_types) {
  std::vector<ColumnDefinition> col;
  col.emplace_back(IntegerType, "col1", make_unique<OrderIndifferentDictionaryFactory>());
  col.emplace_back(IntegerType, "col2", make_unique<OrderPreservingDictionaryFactory>());
  Table t(TableDefinition(10, make_unique<FixedLengthVectorFactory>(), std::move(col)));
  t.resize(2);
  t.setValue<hyrise_int_t>(0, 0, 11);
  t.setValue<hyrise_int_t>(0, 1, 10);
  t.setValue<hyrise_int_t>(1, 0, 10);
  t.setValue<hyrise_int_t>(1, 1, 11);
}



}}
