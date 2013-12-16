// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "MmapStrategy.h"

#include <cstdint>

namespace hyrise {
namespace memory {

MmapStrategy::MmapStrategy(size_t size) :
    _size(size),
    _start((char*)malloc(size)),
    _cur(_start) {}

MmapStrategy::~MmapStrategy() {}

MmapStrategy& MmapStrategy::instance() {
  static auto instance = new MmapStrategy();
  return *instance;
}

void *MmapStrategy::allocate(size_t sz) {
  if (_size - (_cur - _start) < sz)
    throw std::bad_alloc();
  void* ret = _cur;
  _cur += sz;
  return ret;
}

void *MmapStrategy::reallocate(void *old, size_t sz, size_t osz /*old_size*/) {
  if (sz < osz)
    return old;
  else
    return allocate(sz);
}

void MmapStrategy::deallocate(void *p, size_t sz) {}

} } // namespace hyrise::memory

