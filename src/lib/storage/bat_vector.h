// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <tuple>
#include <functional>
#include "storage/BaseAttributeVector.h"



namespace hyrise {
namespace storage {


#define LIKELY(x) __builtin_expect(x, 1)
#define UNLIKELY(x) __builtin_expect(x, 0)

/**
This is the bat vector, not storing most frequent values
*/
template<typename T, bool WithBV = true>
class BATVector : public BaseAttributeVector<T>{

public:

  using value_type = T;
  using pair_type = std::tuple<uint64_t, T>;

  BATVector(const T def, size_t alloc=0): _default(def) {

  }


  /**
  * Append a value
  */
  void push_back(value_type v) {
    if (v != _default) {
      auto s = _size;
      _data.push_back(std::make_tuple(s, v));
    }

    // Only use BV in certain cases
    if (WithBV)
      _nullCheck.push_back(v == _default);

    ++_size;
  }

  value_type get(size_t column, size_t idx) const override {
    // Only when we have the bitvector
    if (WithBV && _nullCheck[idx] ) return _default;

    auto res = std::lower_bound(std::begin(_data), std::end(_data), idx, [](const pair_type& x, const size_t i){
      return std::get<0>(x) < i;
    });

    if (WithBV) {
      return std::get<1>(*res);
    } else {
      if (res != std::end(_data))
        return std::get<1>(*res);
      else
        return _default;
    }
  }

  /**
  Positional access to an element. Since we cant use the index directly we
  have to perform binary search to find the position we are looking for
  */
  value_type operator[](const size_t& idx) const {
    return get(0, idx);
  }

  /**
  * Matching simply scans the entries and yields positions if found
  */
  template<typename Operator>
  std::vector<uint64_t> match(uint64_t start, uint64_t stop, value_type value) {
    Operator op;
    std::vector<uint64_t> result;

    if (_data.size() == 0) return result;

    auto it = _data.cbegin();
    auto pos = std::get<0>(*it);

    while(pos >= start && pos < stop && it != _data.end()) {
      const auto x = *it;
      if (op(std::get<1>(x), value)) {
        result.push_back(std::get<0>(x));
      }
      ++it;
    }
    return result;
  }

  /*
  * Iterate over the values and fill matching positions
  */
  std::vector<uint64_t> anti_match(uint64_t start, uint64_t stop) {
    std::vector<uint64_t> result;
    uint64_t curr = start;
    for (const auto& tuple : _data) {
      if (curr >= stop ) break;

      while(curr < std::get<0>(tuple)) {
        result.push_back(curr++);
      }
    }

    while(curr < _size && curr < stop) {
      result.push_back(curr++);
    }
    return result;
  }


   size_t getColumns() const override {
    return 1;
   }

  size_t size() {
    return _size;
  }

  size_t compressed_size() const {
    return _data.size();
  }

  void set(size_t column, size_t row, T value) override {
    if (value == _default) {
      _nullCheck[row] = true;
      // TODO update handling
    } else {
      _nullCheck[row] = false;
      // Find position to insert
      auto res = std::lower_bound(std::begin(_data), std::end(_data), row, [](const pair_type& x, const size_t i){
        return std::get<0>(x) < i;
      });

      // Insert or update
      if (res == std::end(_data)) {
        _data.insert(res, std::make_tuple(row, value));
      } else {
        (*res) = std::make_tuple(row, value);
      }
    }

    // Ugly bug-ling
    if (row >= _size) {
      _size = row + 1;
    }

  }

  void reserve(size_t rows) override {
    _nullCheck.reserve(rows);
  }

  void resize(size_t rows) override {
    _nullCheck.resize(rows, true);
  }

  uint64_t capacity() override {
    return _nullCheck.capacity();
  }

  void clear() override {
    _data.clear();
    _nullCheck.clear();
  }

  std::shared_ptr<BaseAttributeVector<T>> copy() override {
    return std::make_shared<BATVector<T, true>>(*this);
  }

  void rewriteColumn(const size_t, const size_t) {}

  value_type getDefaultValue() const {
    return _default;
  }

private:

  const value_type _default;

  // Actual data
  std::vector<pair_type> _data;

  // Overall size
  uint64_t _size = 0;

  // bit vector to store if a value is null default
  std::vector<bool> _nullCheck;

};

}
}
