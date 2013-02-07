#include "testing/base_test.h"

#include <algorithm>

#include "helper/RangeIter.h"

namespace hyrise {

class RangeIterTest : public Test {};

TEST_F(RangeIterTest, basic_test) {
  size_t r = std::accumulate(RangeIter(0), RangeIter(5), 0u);
  ASSERT_EQ(r, 0u+1u+2u+3u+4u);
}


}
