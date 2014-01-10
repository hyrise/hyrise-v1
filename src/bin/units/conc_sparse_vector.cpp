// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "gtest/gtest.h"

#include "helper/SparseVector.h"
#include "helper/vector_helpers.h"

TEST(ConcurrentSparseVec, initial) {

  auto t = hyrise::helper::ConcurrentSparseVector<int>(1000, 10);

  ASSERT_EQ(10, t.get(0));
  t.set(0, 12);
  ASSERT_EQ(12, t.get(0));

  ASSERT_EQ(1000u, t.size());
}

TEST(ConcurrentSparseVec, subscript) {

  auto t = hyrise::helper::ConcurrentSparseVector<int>(1000, 10);

  ASSERT_EQ(10, t.get(0));
  t[0] = 12;
  ASSERT_EQ(12, t[0]);
  ASSERT_EQ(1000u, t.size());
}


TEST(ConcurrentSparseVec, iterator ) {
  auto t = hyrise::helper::ConcurrentSparseVector<int>(1000, 10);

  for(const auto& x : t ) {
    ASSERT_EQ(10, x);
  }

  std::vector<int> result(1000,0);
  hyrise::functional::forEachWithIndex(t, [&](size_t i, int k){  result[i] = k; });
  for(const auto& x  : result)
    ASSERT_EQ(10, x);

}

TEST(ConcurrentSparseVec, resizing ) {
  auto t = hyrise::helper::ConcurrentSparseVector<int>(1000, 10);

  for(const auto& x : t ) {
    ASSERT_EQ(10, x);
  }

  ASSERT_ANY_THROW(t.resize(10, 10));

  t.resize(2000);
  ASSERT_EQ(2000u, t.size());

  t.resize(3000, 10);
  ASSERT_EQ(3000u, t.size());

  t.push_back(99);
  ASSERT_EQ(3001u, t.size());
 
}

TEST(ConcurrentSparseVec, cmpxchg ) {
  auto t = hyrise::helper::ConcurrentSparseVector<int>(1000, 10);

  for(const auto& x : t ) {
    ASSERT_EQ(10, x);
  }

  ASSERT_FALSE(t.cmpxchg(7, 88, 10));
  
  ASSERT_TRUE(t.cmpxchg(2,10,88));
  ASSERT_EQ(88, t[2]);

  t[2] = 99;
  ASSERT_TRUE(t.cmpxchg(2,99,88));
  ASSERT_EQ(88, t[2]);
  ASSERT_TRUE(t.cmpxchg(2,88,10));
  ASSERT_EQ(10, t[2]);
  

  t[2] = 99;
  ASSERT_FALSE(t.cmpxchg(2,88,10));
  ASSERT_EQ(99, t[2]);
}
