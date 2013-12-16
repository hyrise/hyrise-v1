// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstdint>

#include "MemoryStrategy.h"

namespace hyrise {
namespace memory {

class MallocStrategy {
private:
  explicit MallocStrategy() {}
  MallocStrategy(const MallocStrategy&) = delete;

public:
  static MallocStrategy& instance() {
    static auto instance = new MallocStrategy();
    return *instance;
  }

  ~MallocStrategy() {}

  virtual void *allocate(size_t sz) {
    return malloc(sz);
  }

  virtual void *reallocate(void *old, size_t sz, size_t osz /*old_size*/) {
    return realloc(old, sz);
  }

  virtual void deallocate(void *p, size_t sz) {
    free(p);
  }
};

} } // namespace hyrise::memory

