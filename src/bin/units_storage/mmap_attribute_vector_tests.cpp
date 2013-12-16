// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <limits>

#include "storage/storage_types.h"
#include "storage/FixedMmapVector.h"

namespace hyrise {
namespace storage {

TEST(FixedMmapVectorTest, increment_test) {
  size_t cols = 1;
  size_t rows = 3;

  FixedMmapVector<value_id_t> tuples(cols, rows);
  tuples.resize(rows);
  EXPECT_EQ(0u, tuples.get(0,0));
  EXPECT_EQ(0u, tuples.inc(0,0));
  EXPECT_EQ(1u, tuples.get(0,0));
  EXPECT_EQ(1u, tuples.atomic_inc(0,0));
  EXPECT_EQ(2u, tuples.get(0,0));
}

} } // namespace hyrise::storage
