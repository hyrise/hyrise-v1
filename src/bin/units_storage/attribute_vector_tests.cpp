// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <storage.h>

#include <storage/BitCompressedVector.h>
#include <storage/FixedLengthVector.h>
#include <storage/AttributeVectorFactory.h>
#include <memory/strategies.h>

template <typename T>
class AttributeVectorTests : public ::hyrise::Test {
public:

};

template<bool Compressed, typename A>
struct TestType {
  static const bool compressed = Compressed;
  typedef A Allocator;
};


TEST( FixedLengthVectorTest, increment_test ) {
  size_t cols = 1;
  size_t rows = 3;

  auto tuples = std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(
    AttributeVectorFactory::getAttributeVector2<value_id_t, StrategizedAllocator<uint, MallocStrategy>>(cols,rows, false));

  tuples->resize(rows);

  EXPECT_EQ(0u, tuples->get(0,0));
  EXPECT_EQ(0u, tuples->inc(0,0));
  EXPECT_EQ(1u, tuples->get(0,0));
  EXPECT_EQ(1u, tuples->atomic_inc(0,0));
  EXPECT_EQ(2u, tuples->get(0,0));
}

static const std::vector<uint64_t> bits = std::vector<uint64_t> {2, 3};

using testing::Types;

typedef Types <
TestType<false, StrategizedAllocator<uint, MallocStrategy> >,
         TestType<true, StrategizedAllocator<uint, MallocStrategy> >
         // BitCompressedVector<uint>,
         // BitCompressedVector<uint, StrategizedAllocator<uint, MemalignStrategy<16> > >,
         // BitCompressedVector<uint, StrategizedAllocator<uint, NumaNodeStrategy<NumaConfig> > >,
         // FixedLengthVector<uint>,
         // FixedLengthVector<uint, StrategizedAllocator<uint, MemalignStrategy<16> > >,
         // FixedLengthVector<uint, StrategizedAllocator<uint, NumaNodeStrategy<NumaConfig> > >
         > Vectors;

TYPED_TEST_CASE(AttributeVectorTests, Vectors);

TYPED_TEST(AttributeVectorTests, boundaries_test) {
  size_t cols = 2;
  size_t rows = 3;
  auto tuples = AttributeVectorFactory::getAttributeVector2<uint32_t, typename TypeParam::Allocator>(cols,
                                                                                                     rows,
                                                                                                     TypeParam::compressed,
                                                                                                     bits);
  tuples->resize(rows);
#ifdef EXPENSIVE_ASSERTIONS
  EXPECT_THROW(tuples->get(cols, 0), std::out_of_range);
  EXPECT_THROW(tuples->get(0, rows), std::out_of_range);
  EXPECT_THROW(tuples->get(cols, rows), std::out_of_range);
#endif
}

TYPED_TEST(AttributeVectorTests, base_test) {

  auto tuples = AttributeVectorFactory::getAttributeVector2<uint32_t, typename TypeParam::Allocator>(1, 3, TypeParam::compressed, bits);
  tuples->resize(3);

  tuples->set(0, 0, 2);
  tuples->set(0, 1, 3);
  tuples->set(0, 2, 1);

  ASSERT_EQ(3u, tuples->size());

  ASSERT_EQ(2u, tuples->get(0, 0));
  ASSERT_EQ(3u, tuples->get(0, 1));
  ASSERT_EQ(1u, tuples->get(0, 2));

  tuples->clear();

  ASSERT_EQ(0u, tuples->size());
}

TYPED_TEST(AttributeVectorTests, two_columns) {
  auto tuples = AttributeVectorFactory::getAttributeVector2<uint32_t, typename TypeParam::Allocator>(2, 3, TypeParam::compressed, bits);
  tuples->resize(3);

  tuples->set(0, 0, 2);
  tuples->set(0, 1, 3);
  tuples->set(0, 2, 1);

  tuples->set(1, 0, 5);
  tuples->set(1, 1, 7);

  ASSERT_EQ(3u, tuples->size());

  ASSERT_EQ(2u, tuples->get(0, 0));
  ASSERT_EQ(3u, tuples->get(0, 1));
  ASSERT_EQ(1u, tuples->get(0, 2));

  ASSERT_EQ(5u, tuples->get(1, 0));
  ASSERT_EQ(7u, tuples->get(1, 1));

  tuples->clear();

  ASSERT_EQ(0u, tuples->size());
}

TYPED_TEST(AttributeVectorTests, copy_test) {

  auto tuples = AttributeVectorFactory::getAttributeVector2<uint32_t, typename TypeParam::Allocator>(2, 3, TypeParam::compressed, bits);
  tuples->resize(3);

  tuples->set(0, 0, 2);
  tuples->set(0, 1, 3);
  tuples->set(0, 2, 1);

  tuples->set(1, 0, 5);
  tuples->set(1, 1, 7);

  std::shared_ptr<BaseAttributeVector<uint32_t>> copy = tuples->copy();
  tuples->clear();

  ASSERT_EQ(3u, copy->size());

  ASSERT_EQ(2u, copy->get(0, 0));
  ASSERT_EQ(3u, copy->get(0, 1));
  ASSERT_EQ(1u, copy->get(0, 2));

  ASSERT_EQ(5u, copy->get(1, 0));
  ASSERT_EQ(7u, copy->get(1, 1));

  copy->clear();

  ASSERT_EQ(0u, copy->size());
}

TYPED_TEST(AttributeVectorTests, empty_does_not_fail_when_reserve_test) {
  auto tuples = AttributeVectorFactory::getAttributeVector2<uint32_t, typename TypeParam::Allocator>(1, 0, TypeParam::compressed, bits);
  tuples->resize(3);

  // when adding three elements
  tuples->set(0, 0, 2);
  tuples->set(0, 1, 3);
  tuples->set(0, 2, 1);

  ASSERT_EQ(3u, tuples->size());

  ASSERT_EQ(2u, tuples->get(0, 0));
  ASSERT_EQ(3u, tuples->get(0, 1));
  ASSERT_EQ(1u, tuples->get(0, 2));
}

TYPED_TEST(AttributeVectorTests, empty_size_does_not_change_with_reserve) {

  auto tuples = AttributeVectorFactory::getAttributeVector2<uint32_t, typename TypeParam::Allocator>(1, 1, TypeParam::compressed, std::vector<uint64_t> {1});
  ASSERT_EQ(0u, tuples->size());
  if (TypeParam::compressed)
    ASSERT_EQ(64u, tuples->capacity());
  else
    ASSERT_EQ(1u, tuples->capacity());
  tuples->reserve(3);
  ASSERT_EQ(0u, tuples->size());
  if (TypeParam::compressed)
    ASSERT_EQ(64u, tuples->capacity());
  else
    ASSERT_EQ(3u, tuples->capacity());

}

