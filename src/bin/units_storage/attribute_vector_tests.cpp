// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <limits>

#include "storage/storage_types.h"
#include "storage/BitCompressedVector.h"
#include "storage/FixedLengthVector.h"

namespace hyrise {
namespace storage {

TEST(BitCompressedTests, set_retrieve_bits) {
  std::vector<uint64_t> bits {1, 2, 4, 8, 13};
  BitCompressedVector<value_id_t> tuples(5, 2, bits);
  tuples.resize(2);
  auto col = 0;
  for (auto bit: bits) {
    auto maxval = (1 << bit) - 1; // Maximum value that can be stored is 2^bit-1
    tuples.set(col, 0, 0);
    tuples.set(col, 1, maxval);
    EXPECT_EQ(0u, tuples.get(col, 0));
    EXPECT_EQ(maxval, tuples.get(col, 1));
    col++;
  }
}

TEST(BitCompressedTests, empty_size_does_not_change_with_reserve) {
  BitCompressedVector<value_id_t> tuples(1, 1, {1});
  ASSERT_EQ(0u, tuples.size());
  ASSERT_EQ(64u, tuples.capacity());
  tuples.reserve(3);
  ASSERT_EQ(0u, tuples.size());
  ASSERT_EQ(64u, tuples.capacity());
  tuples.reserve(65);
  ASSERT_EQ(0u, tuples.size());
  ASSERT_EQ(128u, tuples.capacity());
}

TEST(FixedLengthVectorTest, increment_test) {
  size_t cols = 1;
  size_t rows = 3;

  FixedLengthVector<value_id_t> tuples(cols, rows);
  tuples.resize(rows);
  EXPECT_EQ(0u, tuples.get(0,0));
  EXPECT_EQ(0u, tuples.inc(0,0));
  EXPECT_EQ(1u, tuples.get(0,0));
  EXPECT_EQ(1u, tuples.atomic_inc(0,0));
  EXPECT_EQ(2u, tuples.get(0,0));
}

} } // namespace hyrise::storage
