// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstdint>

namespace hyrise {
namespace memory {

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

} } // namespace hyrise::memory

