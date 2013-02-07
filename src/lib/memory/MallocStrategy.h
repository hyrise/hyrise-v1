#ifndef SRC_LIB_MEMORY_MALLOCSTRATEGY_H_
#define SRC_LIB_MEMORY_MALLOCSTRATEGY_H_

#include <iostream>
#include <cstdint>
#include <cstring>

class MallocStrategy {

public:
  static uint64_t   allocated;
  static uint64_t deallocated;


  static void *allocate(size_t sz) {
    return malloc(sz);
  }

  static void *reallocate(void *old, size_t sz, size_t osz /*old_size*/) {
    return realloc(old, sz);
  }

  static void deallocate(void *p, size_t sz) {
    free(p);
  }
};

#endif  // SRC_LIB_MEMORY_MALLOCSTRATEGY_H_

