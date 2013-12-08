#include "testing/test.h"

#include <limits>

#include "storage/storage_types.h"
#include "storage/BitCompressedVector.h"
#include "storage/FixedLengthVector.h"
#include "storage/ConcurrentFixedLengthVector.h"
#include "storage/AttributeVectorFactory.h"

namespace hyrise {
namespace storage {

template <typename T>
class AttributeVectorTests : public ::hyrise::Test {};

typedef testing::Types <
  BitCompressedVector<value_id_t>,
  FixedLengthVector<value_id_t>,
  ConcurrentFixedLengthVector<value_id_t>
  > Vectors;

TYPED_TEST_CASE(AttributeVectorTests, Vectors);

template <typename T>
void insertVals(T& tuples, std::size_t cols, std::size_t rows) {
  value_id_t vid = 0;
  tuples.resize(rows);
  for (std::size_t r=0; r < rows; ++r) {
    for (std::size_t c=0; c < cols; ++c) {
      tuples.set(c, r, vid++);
    }
  }
}

namespace {
size_t cols = 2;
size_t rows = 3;
}

TYPED_TEST(AttributeVectorTests, boundaries_test) {
  TypeParam tuples(cols, rows);
  insertVals(tuples, cols, rows);
#ifdef EXPENSIVE_ASSERTIONS
  EXPECT_THROW(tuples.set(2, 0, 1), std::out_of_range) << "Beyond column boundary";
  EXPECT_THROW(tuples.set(0, 3, 1), std::out_of_range) << "Beyond row boundary";
  EXPECT_THROW(tuples.set(3, 4, 1), std::out_of_range) << "Beyond row/column boundary";
#endif
}

TYPED_TEST(AttributeVectorTests, insert_retrieve_max) {
  TypeParam tuples(cols, rows);
  tuples.resize(10);
  ASSERT_LE(10, tuples.size());
  value_id_t maxval = std::numeric_limits<value_id_t>::max();
  tuples.set(0, 0, maxval);
  ASSERT_EQ(maxval, tuples.get(0, 0));
}

TYPED_TEST(AttributeVectorTests, copy) {
  TypeParam tuples(cols, rows);
  insertVals(tuples, cols, rows);
  auto cp = tuples.copy();
  ASSERT_EQ(tuples.size(), cp->size());
}

} } // namespace hyrise::storage

