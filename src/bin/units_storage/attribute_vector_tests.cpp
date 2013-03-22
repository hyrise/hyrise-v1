// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <storage.h>

#include <storage/BitCompressedVector.h>
#include <storage/FixedLengthVector.h>
#include <storage/AttributeVectorFactory.h>
#include <memory/strategies.h>

#include <storage/DefaultDictVector.hpp>


#include <iostream>

template <typename T>
class AttributeVectorTests : public ::hyrise::Test {
public:

};

template<bool Compressed, typename A>
struct TestType {
  static const bool compressed = Compressed;
  typedef A Allocator;
};


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

TYPED_TEST(AttributeVectorTests, default_bit_vector) {

  #define MAX 100000
  #define COLS 4
  DefaultDictVector< hyrise_int_t > d(COLS,10);
  //FixedLengthVector< hyrise_int_t > f(4,10);
  
  int i = 0;
  int j = 0;

  d.resize(9);

  for (j = 0; j<d.size();++j)
    for (i = 0; i<COLS; ++i)
      d.set(i,j,(i*COLS+j)*((i+j)%2));
  
  for (j = 0; j<d.size();++j)
    for (i = 0; i<COLS; ++i)
      ASSERT_EQ(d.get(i,j),(i*COLS+j)*((i+j)%2));
  



  //for (DefaultDictVectorIterator<hyrise_int_t> it = d.begin(); it != d.end(); ++it)  {
  //  ++i;
 // }
 // ASSERT_EQ(0,i);

 // d.push_back(true);
 // for (DefaultDictVectorIterator<hyrise_int_t> it = d.begin(); it != d.end(); ++it)  {
 //   ASSERT_EQ(1,*it);
 //   ++i;
 // }
 // ASSERT_EQ(1,i);

/*
  for(i=1;i<MAX;++i){
    int nb = i % 10;
    if(nb == 5) {
      d.push_back(false);
    } else {
      d.push_back(true);
    }
  }

  for(i=0;i<(long)d.size();++i) {
    ASSERT_EQ(hyrise_int_t((i%10)!=5),d[i]);
    ASSERT_EQ(d[i],*d.iterator(i));
  }

  i = 0;
  for (DefaultDictVectorIterator<hyrise_int_t> it = d.begin(); it != d.end(); ++it) {
    ASSERT_EQ(d[i],*it);
    ++i;
  }

  ASSERT_EQ(MAX,i);


  i = 0;
  for (DefaultDictVectorIterator<hyrise_int_t> it = d.begin(); it != d.end(); ++it) {
    ASSERT_EQ(d[i],*it);
    ++i;
  }*/
}
