#include "testing/test.h"

#include <storage.h>
#include <storage/BitCompressedVector.h>

class MockStrategyTest : public ::hyrise::Test {};


class MockMallocStrategy {
public:
  static int allocates;
  static int reallocates;
  static int deallocates;
  static void *allocate(size_t sz) {
    ++allocates;
    return malloc(sz);
  }

  static void *reallocate(void *old, size_t sz, size_t /*old_size*/) {
    ++reallocates;
    return realloc(old, sz);
  }

  static void deallocate(void *p, size_t sz) {
    ++deallocates;
    free(p);
  }
};

int MockMallocStrategy::allocates = 0;
int MockMallocStrategy::deallocates = 0;
int MockMallocStrategy::reallocates = 0;


typedef BitCompressedVector<uint, StrategizedAllocator<uint, MockMallocStrategy> > bum;
TEST_F(MockStrategyTest, base_test) {
  auto e = std::vector<uint64_t> {1};
  bum *a = new bum(1, 10, e);
  delete a;
  ASSERT_EQ(MockMallocStrategy::allocates + MockMallocStrategy::reallocates, MockMallocStrategy::deallocates);
}
