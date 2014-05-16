// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

namespace hyrise {
namespace storage {

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
class BATVector {

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

/**
Positional access to an element. Since we cant use the index directly we
have to perform binary search to find the position we are looking for
*/
value_type operator[](const size_t& idx) const {

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

size_t size() const {
  return _size;
}

size_t compressed_size() const {
  return _data.size();
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
