// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstdint>
#include <memory>

#define GIGABYTE(x) (MEGABYTE(x) * 1024)
#define MEGABYTE(x) (KILOBYTE(x) * 1024)
#define KILOBYTE(x) (x * 1024)

namespace hyrise {
namespace memory {

class MemoryStrategy {
public:
  virtual ~MemoryStrategy() {}

  virtual void *allocate(size_t sz) = 0;
  virtual void *reallocate(void *old, size_t sz, size_t osz /*old_size*/) = 0;
  virtual void deallocate(void *p, size_t sz) = 0;
};

} } // namespace hyrise::memory

