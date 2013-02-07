// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_FIXEDLENGTHVECTOR_H_
#define SRC_LIB_STORAGE_FIXEDLENGTHVECTOR_H_

#include <errno.h>
#include <math.h>

#include <memory>
#include <mutex>
#include <string>
#include <stdexcept>

#include <boost/iterator/iterator_facade.hpp>

#include <storage/AbstractAttributeVector.h>
#include <storage/BaseAttributeVector.h>
#include <storage/BaseAllocatedAttributeVector.h>
#include <memory/StrategizedAllocator.h>
#include <memory/MallocStrategy.h>

template<typename T>
class FixedLengthVectorIterator;

//template <typename T, typename Allocator = StrategizedAllocator<T, MemalignStrategy<4096> > >
template <typename T, typename Allocator = StrategizedAllocator<T, MallocStrategy > >
class FixedLengthVector : public BaseAllocatedAttributeVector< FixedLengthVector<T, Allocator>, Allocator> {
private:
  T *_values;
  size_t _rows;
  size_t _columns;
  size_t _allocated_bytes;

  std::mutex _allocate_mtx;

public:
  explicit FixedLengthVector(size_t columns,
                             size_t rows);

  virtual ~FixedLengthVector();

  void *data() {
    return _values;
  };

  void setNumRows(size_t s) {
    _rows = s;
  }

  T get(size_t column, size_t row) const;

  const T& getRef(size_t column, size_t row) const;

  void set(size_t column, size_t row, T value);

  void reserve(size_t rows);

  void clear();

  size_t size();

  void resize(size_t rows) {
    reserve(rows);
    _rows = rows;
  }

  void rewriteColumn(const size_t column, const size_t bits) {}

  // returns the capacity of the container
  inline uint64_t capacity() {
    return _allocated_bytes / (sizeof(value_type) * _columns);
  }

  std::shared_ptr<BaseAttributeVector<T>> copy();

  typedef T value_type;

private:
  void allocate(size_t bytes);

  inline void checkAccess(const size_t& column, const size_t& rows) const {
#ifdef EXPENSIVE_ASSERTIONS
    if (column >= _columns) {
      throw std::out_of_range("Trying to access column '"
                              + std::to_string(column) + "' where only '"
                              + std::to_string(_columns) + "' available");
    }
    if (rows >= _rows) {
      throw std::out_of_range("Trying to access rows '"
                              + std::to_string(rows) + "' where only '"
                              + std::to_string(_rows) + "' available");
    }
#endif
  }
};

template <typename T, typename Allocator>
FixedLengthVector<T, Allocator>::FixedLengthVector(size_t columns, size_t rows) :
    _values(nullptr), _rows(0), _columns(columns), _allocated_bytes(0) {
  if (rows > 0) {
    reserve(rows);
  }
}

template <typename T, typename Allocator>
FixedLengthVector<T, Allocator>::~FixedLengthVector() {
  Allocator::Strategy::deallocate(_values, _allocated_bytes);
}

template <typename T, typename Allocator>
T FixedLengthVector<T, Allocator>::get(size_t column, size_t row) const {
  checkAccess(column, row);
  return _values[row * _columns + column];
}

template <typename T, typename Allocator>
const T& FixedLengthVector<T, Allocator>::getRef(size_t column, size_t row) const {
  checkAccess(column, row);
  return _values[row * _columns + column];
}


template <typename T, typename Allocator>
void FixedLengthVector<T, Allocator>::set(size_t column, size_t row, T value) {
  checkAccess(column, row);
  _values[row * _columns + column] = value;
}

template <typename T, typename Allocator>
void FixedLengthVector<T, Allocator>::reserve(size_t rows) {
  allocate(_columns * rows * sizeof(T));
}

template <typename T, typename Allocator>
void FixedLengthVector<T, Allocator>::clear() {
  allocate(8);
  _rows = 0;
}

template <typename T, typename Allocator>
size_t FixedLengthVector<T, Allocator>::size() {
  return _rows;
}

template <typename T, typename Allocator>
void FixedLengthVector<T, Allocator>::allocate(size_t bytes) {
  std::lock_guard<std::mutex> guard(_allocate_mtx);

  if (bytes != _allocated_bytes) {
    void *new_values = Allocator::Strategy::reallocate(_values, bytes, _allocated_bytes);

    if (new_values == nullptr) {
      ///std::cerr << strerror(errno) << std::endl;
      Allocator::Strategy::deallocate(_values, _allocated_bytes);
      throw std::bad_alloc();
    }

    _values = static_cast<T*>(new_values);
    _allocated_bytes = bytes;
  }
}

template <typename T, typename Allocator>
std::shared_ptr<BaseAttributeVector<T>> FixedLengthVector<T, Allocator>::copy() {
  auto copy = std::make_shared<FixedLengthVector<T, Allocator>>(_columns, _rows);
  copy->_rows = _rows;
  copy->allocate(_allocated_bytes);
  memcpy(copy->_values, _values, _allocated_bytes);

  return copy;
}

#endif  // SRC_LIB_STORAGE_FIXEDLENGTHVECTOR_H_
