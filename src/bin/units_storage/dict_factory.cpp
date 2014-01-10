#include "gtest/gtest.h"

#include "storage/DictionaryFactory.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/OrderIndifferentDictionary.h"

namespace hyrise { namespace storage {

TEST(factory, creation) {
  auto d = makeDictionary(IntegerType, 10);
}

}}
