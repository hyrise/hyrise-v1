// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "memory/MallocStrategy.h"
#include "memory/MemalignStrategy.h"
#include "memory/NumaStrategy.h"
#include "memory/NumaStrategy2.h"

template <typename T>
class StrategyTests : public ::hyrise::Test {};

using testing::Types;
typedef Types<MallocStrategy, MemalignStrategy<16>, MemalignStrategy<64>, NumaNodeStrategy<NumaConfig>, NumaNodeStrategy2<NumaConfig> > Strategies;


TYPED_TEST_CASE(StrategyTests, Strategies);

TYPED_TEST(StrategyTests, basic_alloc) {
  void *p = TypeParam::allocate(10 * sizeof(int));
  ASSERT_TRUE(p != nullptr);
  TypeParam::deallocate(p, 10 * sizeof(int));
}

TYPED_TEST(StrategyTests, basic_realloc) {
  void *p = TypeParam::allocate(10 * sizeof(int));
  ASSERT_TRUE(p != nullptr);

  p = TypeParam::reallocate(p, 100 * sizeof(int), 10 * sizeof(int));
  ASSERT_TRUE(p != nullptr);

  TypeParam::deallocate(p, 100 * sizeof(int));
}

TYPED_TEST(StrategyTests, vector_assignment_diff) {
  std::vector<int, StrategizedAllocator<int, TypeParam> > a;
  std::vector<int, StrategizedAllocator<int, TypeParam> > b;

  a.push_back(10);
  a.push_back(20);

  b.push_back(15);
  b.push_back(25);

  assign(a, b);

  ASSERT_EQ(a[0], 15);
  ASSERT_EQ(a[1], 25);
}

TYPED_TEST(StrategyTests, vector_assignment_same) {
  std::vector<int> a;
  std::vector<int, StrategizedAllocator<int, TypeParam> > b;

  a.push_back(10);
  a.push_back(20);

  b.push_back(15);
  b.push_back(25);

  assign(a, b);

  ASSERT_EQ(a[0], 15);
  ASSERT_EQ(a[1], 25);
}
