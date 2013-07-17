// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_MEMORY_MALLOCSTRATEGY_H_
#define SRC_LIB_MEMORY_MALLOCSTRATEGY_H_

#include <iostream>
#include <cstdint>


class MallocStrategy {
public:
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

