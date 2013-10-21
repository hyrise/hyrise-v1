// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "gtest/gtest.h"

#include <algorithm>

#include "helper/RangeIter.h"

TEST(RangeIterTest, basic_test) {
  size_t r = std::accumulate(RangeIter(0), RangeIter(5), 0u);
  ASSERT_EQ(r, 0u+1u+2u+3u+4u);
}
