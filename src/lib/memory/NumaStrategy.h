#ifndef SRC_LIB_MEMORY_NUMASTRATEGY_H_
#define SRC_LIB_MEMORY_NUMASTRATEGY_H_

#include <stdlib.h>
#include <iostream>

#ifdef WITH_NUMA
#include <numa.h>
#endif

class NumaConfig {
 public:
  static const int node = 0;
};



template<typename config>
class NumaNodeStrategy {
 public:
#ifdef WITH_NUMA
  static void *allocate(const size_t sz) {
    numa_set_bind_policy(1);
    numa_set_preferred(config::node);
    void *r = numa_alloc(sz); //numa_alloc_onnode(sz, config::node);
    numa_set_preferred(-1);
    numa_set_bind_policy(0);
    return r;
  };


  static void *reallocate(void *old, size_t sz, size_t old_sz) {
    if (old == nullptr)
      return allocate(sz);
    else
      return numa_realloc(old, old_sz, sz);
  };

  static void deallocate(void *p, size_t sz) {
    numa_free(p, sz);
  };
#else
  static void *allocate(size_t sz) {
    return malloc(sz);
  };

  static void *reallocate(void *old, size_t sz, size_t /*old_sz*/) {
    return realloc(old, sz);
  };

  static void deallocate(void *p, size_t sz) {
    free(p);
  };
#endif //WITH_NUMA
};

#endif  // SRC_LIB_MEMORY_NUMASTRATEGY_H_
