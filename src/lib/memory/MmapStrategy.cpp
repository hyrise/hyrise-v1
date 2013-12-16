// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "MmapStrategy.h"

#include <cstdint>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace hyrise {
namespace memory {

std::string uniqueFileName() {
  static int i = 0;
  std::ostringstream ss;
  ss << '.' << i++ << ".mmap";
  return ss.str();
}

void *allocateMmapMemory(size_t size, int fd) {
  if (fd == -1)
    throw std::bad_alloc();

  int result = lseek(fd, size - 1, SEEK_SET);
  if (result == -1) {
    close(fd);
    throw std::bad_alloc();
  }

  result = write(fd, "", 1);
  if (result != 1) {
    close(fd);
    throw std::bad_alloc();
    exit(EXIT_FAILURE);
  }

  void *map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    throw std::bad_alloc();
    exit(EXIT_FAILURE);
  }

  return map;
}

MmapStrategy::MmapStrategy(size_t size) :
    _size(size),
    _fileName(uniqueFileName()),
    _fd(open(_fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600)),
    _start(static_cast<char*>(allocateMmapMemory(size, _fd))),
    _cur(_start) {
}

MmapStrategy::~MmapStrategy() {
  if (munmap(_start, _size) == -1)
    throw std::bad_alloc();

  close(_fd);
  remove(_fileName.c_str());
}

MmapStrategy& MmapStrategy::instance() {
  static MmapStrategy instance;
  return instance;
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

