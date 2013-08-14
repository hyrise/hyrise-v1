// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_FIXEDLENGTHVECTOR_H_
#define SRC_LIB_STORAGE_FIXEDLENGTHVECTOR_H_


#include <cerrno>
#include <cstring>
#include <cmath>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <stdexcept>
#include <sstream>

#include "memory/MallocStrategy.h"
#include "storage/BaseAttributeVector.h"

template <typename T>
class FixedLengthVector : public BaseAttributeVector<T> {
 private:
  T *_values;
  size_t _rows;
  size_t _columns;
  size_t _allocated_bytes;

  std::mutex _allocate_mtx;
  using Strategy = MallocStrategy;
 public:
  typedef T value_type;
  
  FixedLengthVector(size_t columns,  size_t rows)  :
      _values(nullptr), _rows(0), _columns(columns), _allocated_bytes(0) {
    if (rows > 0) {
      reserve(rows);
    }
  }

  virtual ~FixedLengthVector() {
    Strategy::deallocate(_values, _allocated_bytes);
  }

  void *data() {
    return _values;
  }

  void setNumRows(size_t s) {
    _rows = s;
  }

  inline T get(size_t column, size_t row) const {
    checkAccess(column, row);
    return _values[row * _columns + column];
  }

  const T& getRef(size_t column, size_t row) const {
    checkAccess(column, row);
    return _values[row * _columns + column];
  }

  inline void set(size_t column, size_t row, T value) {
    checkAccess(column, row);
    _values[row * _columns + column] = value;
  }

  void reserve(size_t rows) {
    allocate(_columns * rows * sizeof(T));
  }

  void clear() {
    allocate(8);
    _rows = 0;
  }

  size_t size() {
    return _rows;
  }

  void resize(size_t rows) {
    reserve(rows);
    _rows = rows;
  }

  void rewriteColumn(const size_t column, const size_t bits) {}

  // returns the capacity of the container
  inline uint64_t capacity() {
    return _allocated_bytes / (sizeof(value_type) * _columns);
  }

  std::shared_ptr<BaseAttributeVector<T>> copy() {
    auto copy = std::make_shared<FixedLengthVector<T>>(_columns, _rows);
    copy->_rows = _rows;
    copy->allocate(_allocated_bytes);
    memcpy(copy->_values, _values, _allocated_bytes);
    return copy;
  }

  // Increment the value by 1
  T inc(size_t column, size_t row) {
    checkAccess(column, row);
    return _values[row * _columns + column]++;
  }

  // Atomic Increment the value by one
  T atomic_inc(size_t column, size_t row) {
    checkAccess(column, row);
    return __sync_fetch_and_add(&_values[row * _columns + column], 1);
  }


  const std::string print() {
    std::stringstream buf;
    buf << "Table: " << this << " --- " << std::endl;
    for(size_t i=0; i < size(); ++i) {
      buf << "| ";
      for(size_t j=0; j < _columns; ++j)
        buf << get(j, i) << " |";
      buf << std::endl;
    }
    buf << this << " ---" << std::endl;;
    return buf.str();
  }



 private:
  void allocate(size_t bytes) {
    std::lock_guard<std::mutex> guard(_allocate_mtx);

    if (bytes != _allocated_bytes) {
      void *new_values = Strategy::reallocate(_values, bytes, _allocated_bytes);
    
      if (bytes > _allocated_bytes)
        memset(((char*) new_values) + _allocated_bytes, 0, bytes - _allocated_bytes);

      if (new_values == nullptr) {
        Strategy::deallocate(_values, _allocated_bytes);
        throw std::bad_alloc();
      }

      _values = static_cast<T*>(new_values);
      _allocated_bytes = bytes;
    }

  }

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

#endif  // SRC_LIB_STORAGE_FIXEDLENGTHVECTOR_H_
