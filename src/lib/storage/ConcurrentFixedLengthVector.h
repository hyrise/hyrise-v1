#pragma once

#include "storage/FixedLengthVector.h"
#include "helper/not_implemented.h"
#include "tbb/concurrent_vector.h"

namespace hyrise {
namespace storage {

template <typename T>
class ConcurrentFixedLengthVector final : public AbstractFixedLengthVector<T> {
 public:
  ConcurrentFixedLengthVector(std::size_t columns, std::size_t rows)
      : _columns(columns), _size(rows), _values(columns * rows) {}

  // Increment the value by 1
  virtual T inc(size_t column, size_t row) override {
    check_access(column, row);
    return _values.at(row * _columns + column)++;
  }

  // Atomic Increment the value by one
  T atomic_inc(size_t column, size_t row) {
    check_access(column, row);
    return __sync_fetch_and_add(&_values.at(row * _columns + column), 1);
  }

  virtual T get(size_t column, size_t row) const override { return getRef(column, row); }

  virtual const T& getRef(size_t column, size_t row) const override {
    check_access(column, row);
    return _values[row * _columns + column];
  }

  virtual void set(size_t column, size_t row, T value) override {
    check_access(column, row);
    _values[row * _columns + column] = value;
  }

  virtual void reserve(size_t rows) override { _values.grow_to_at_least(_columns * rows); }

  virtual void resize(size_t rows) override {
    reserve(rows);
    _size = rows;
  }

  virtual std::uint64_t capacity() override { return _values.capacity() / _columns; }

  virtual std::uint64_t size() override { return _size; }

  size_t getColumns() const override { return _columns; }

  virtual std::shared_ptr<BaseAttributeVector<T>> copy() override {
    return std::make_shared<ConcurrentFixedLengthVector>(*this);
  }

  virtual void clear() { NOT_IMPLEMENTED }
  virtual void rewriteColumn(const size_t, const size_t) {}

 private:
  void check_access(std::size_t columns, std::size_t rows) const {
#ifdef EXPENSIVE_ASSERTIONS
    if (columns >= _columns) {
      throw std::out_of_range("Accessing column beyond boundaries");
    }
    if (rows >= (_values.size() / _columns)) {
      throw std::out_of_range("Accessing row beyond boundaries");
    }
#endif
  }
  const std::size_t _columns;
  size_t _size;
  tbb::concurrent_vector<T> _values;
};
}
}  // namespace hyrise::storage
