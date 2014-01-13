// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <atomic>
#include <memory>
#include <stdexcept>
#include <vector>


#include "helper/not_implemented.h"
#include "storage/BaseAttributeVector.h"

namespace hyrise {
namespace storage {

template <typename T>
class AbstractFixedLengthVector : public BaseAttributeVector<T> {
 public:
  virtual const T& getRef(size_t column, size_t row) const = 0;
};

template <typename T>
class FixedLengthVector : public AbstractFixedLengthVector<T> {
 public:
  FixedLengthVector(std::size_t columns, std::size_t rows) :
      _columns(columns), _values(columns * rows) {}

  // Increment the value by 1
  T inc(size_t column, size_t row) {
    check_access(column, row);
    return _values[row * _columns + column]++;
  }

  // Atomic Increment the value by one
  T atomic_inc(size_t column, size_t row) {
    check_access(column, row);
    return __sync_fetch_and_add(&_values[row * _columns + column], 1);
  }

  virtual T get(size_t column, size_t row) const override {
    return getRef(column, row);
  }

  virtual const T& getRef(size_t column, size_t row) const override {
    check_access(column, row);
    return _values[row * _columns + column];
  }

  virtual void set(size_t column, size_t row, T value) override {
    check_access(column, row);
    _values[row * _columns + column] = value;
  }

  virtual void reserve(size_t rows) override {
    _values.resize(_columns * rows);
  }

  virtual void resize(size_t rows) override {
    reserve(rows);
  }

  virtual std::uint64_t capacity() override {
    return _values.capacity() / _columns;
  }

  virtual std::uint64_t size() override {
    return _values.size() / _columns;
  }

  virtual void setNumRows(std::size_t num) override { NOT_IMPLEMENTED }

  virtual std::shared_ptr<BaseAttributeVector<T>> copy() override {
    return std::make_shared<FixedLengthVector>(*this);
  }

  virtual void clear() { _values.clear(); }
  virtual void rewriteColumn(const size_t, const size_t) {}
  virtual void *data() override { return _values.data();}
 private:
  void check_access(std::size_t columns, std::size_t rows) const {
#ifdef EXPENSIVE_ASSERTIONS
    if (columns >= _columns) { throw std::out_of_range("Accessing column beyond boundaries"); }
    if (rows >= (_values.size() / _columns)) { throw std::out_of_range("Accessing row beyond boundaries"); }
#endif
  }
  const std::size_t _columns;
  std::vector<T> _values;
};


} } // namespace hyrise::storage
