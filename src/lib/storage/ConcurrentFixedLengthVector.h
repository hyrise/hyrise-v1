#pragma once

#include "storage/FixedLengthVector.h"
#include "helper/not_implemented.h"
#include "tbb/concurrent_vector.h"

namespace hyrise {
namespace storage {

template <typename T>
class ConcurrentFixedLengthVector : public AbstractFixedLengthVector<T> {
 public:
  ConcurrentFixedLengthVector(std::size_t columns, std::size_t rows) :
      _columns(columns), _values(columns * rows) {}

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
    _values.grow_to_at_least(_columns * rows);
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
    return std::make_shared<ConcurrentFixedLengthVector>(*this);
  }

  virtual void clear() {NOT_IMPLEMENTED}
  virtual void rewriteColumn(const size_t, const size_t) {NOT_IMPLEMENTED}
  virtual void *data() override {NOT_IMPLEMENTED}
 private:
  void check_access(std::size_t columns, std::size_t rows) const {
#ifdef EXPENSIVE_ASSERTIONS
    if (columns >= _columns) { throw std::out_of_range("Accessing column beyond boundaries"); }
    if (rows >= (_values.size() / _columns)) { throw std::out_of_range("Accessing row beyond boundaries"); }
#endif
  }
  const std::size_t _columns;
  tbb::concurrent_vector<T> _values;
};

} } // namespace hyrise::storage

