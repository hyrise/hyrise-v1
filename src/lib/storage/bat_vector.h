// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

namespace hyrise {
namespace storage {

#include "storage/BaseAttributeVector.h"


#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <tuple>


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

}

void reserve(size_t rows) override {

}

void resize(size_t rows) override {

}

uint64_t capacity() override {
  return 0;
}

void clear() override {
  _data.clear();
  _nullCheck.clear();
}

std::shared_ptr<BaseAttributeVector<T>> copy() override {
  return std::make_shared<BATVector<T, true>>(*this);
}

void rewriteColumn(const size_t, const size_t) {}

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
