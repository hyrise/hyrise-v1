// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstdint>

#include "MemoryStrategy.h"

namespace hyrise {
namespace memory {

class MmapStrategy : public MemoryStrategy {
private:
  explicit MmapStrategy(size_t size = MEGABYTE(50));
  MmapStrategy(const MmapStrategy&) = delete;

  virtual ~MmapStrategy();

const size_t _size;
const std::string _fileName;
const int _fd;
char* _start;
char* _cur;

public:
  static MmapStrategy& instance();

  virtual void *allocate(size_t sz);
  virtual void *reallocate(void *old, size_t sz, size_t osz /*old_size*/);
  virtual void deallocate(void *p, size_t sz);
};

} } // namespace hyrise::memory

