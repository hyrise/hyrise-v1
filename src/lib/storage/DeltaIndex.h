// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <iostream>

#include "tbb/concurrent_vector.h"
#include "tbb/concurrent_hash_map.h"

#include "helper/types.h"
#include "helper/locking.h"

#include "storage/storage_types.h"
#include "storage/AbstractIndex.h"
#include "storage/AbstractTable.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>

namespace hyrise {
namespace storage {


// An inverted index for the delta that can be queried and
// is modifiable to add new values. However, the structure
// is not threadsafe and needs to be synchronized via
// read_lock and write_lock.
// Individual pos_lists for one key are stored sorted.
template <typename T>
class DeltaIndex : public AbstractIndex {
 private:
  typedef std::multimap<T, pos_t> inverted_index_t;
  typedef std::pair<typename inverted_index_t::iterator, typename inverted_index_t::iterator> range_t;

  inverted_index_t _index;
  RWMutex _mtx;

 public:
  virtual ~DeltaIndex() {}

  void shrink() { throw std::runtime_error("Shrink not supported for DeltaIndex"); }

  void write_lock() { _mtx.lock_exclusive(); }

  void unlock() { _mtx.unlock(); }

  explicit DeltaIndex(std::string id = "volatile_delta_index", size_t capacity = 1000000) {}

  void add(T value, pos_t pos) {
    _index.insert({value, pos});
  };

  static void copy(typename inverted_index_t::iterator first,
                   typename inverted_index_t::iterator last,
                   std::shared_ptr<pos_list_t> result) {
    for (auto it = first; it != last; it++) {
      result->push_back(it->second);
    }
  }

  range_t getIteratorsForKey(T key) {
    SharedLock sl(_mtx);
    return _index.equal_range(key);
  }

  range_t getIteratorsForKeyBetween(T key, T upper) {
    SharedLock sl(_mtx);
    auto l = _index.lower_bound(key);
    auto u = _index.upper_bound(upper);
    return {l, u};
  }

  PositionRange getPositionsForKey(T key) {
    std::shared_ptr<pos_list_t> pos_list(new pos_list_t);
    {
      SharedLock sl(_mtx);
      auto range = _index.equal_range(key);
      copy(range.first, range.second, pos_list);
    }
    return PositionRange(pos_list->cbegin(), pos_list->cend(), false, pos_list);
  };

  PositionRange getPositionsForKeyLT(T key) {
    std::shared_ptr<pos_list_t> pos_list(new pos_list_t);
    {
      SharedLock sl(_mtx);
      copy(_index.begin(), _index.lower_bound(key), pos_list);
    }
    return PositionRange(pos_list->cbegin(), pos_list->cend(), false, pos_list);
  };

  PositionRange getPositionsForKeyLTE(T key) {
    std::shared_ptr<pos_list_t> pos_list(new pos_list_t);
    {
      SharedLock sl(_mtx);
      copy(_index.begin(), _index.upper_bound(key), pos_list);
    }
    return PositionRange(pos_list->cbegin(), pos_list->cend(), false, pos_list);
  };

  PositionRange getPositionsForKeyBetween(T a, T b) {
    // return range [a,b]
    if (a > b)
      std::swap(a, b);
    std::shared_ptr<pos_list_t> pos_list(new pos_list_t);
    {
      SharedLock sl(_mtx);
      copy(_index.lower_bound(a), _index.upper_bound(b), pos_list);
    }
    return PositionRange(pos_list->cbegin(), pos_list->cend(), false, pos_list);
  };

  PositionRange getPositionsForKeyGT(T key) {
    std::shared_ptr<pos_list_t> pos_list(new pos_list_t);
    {
      SharedLock sl(_mtx);
      copy(_index.upper_bound(key), _index.end(), pos_list);
    }
    return PositionRange(pos_list->cbegin(), pos_list->cend(), false, pos_list);
  };

  PositionRange getPositionsForKeyGTE(T key) {
    std::shared_ptr<pos_list_t> pos_list(new pos_list_t);
    {
      SharedLock sl(_mtx);
      copy(_index.lower_bound(key), _index.end(), pos_list);
    }
    return PositionRange(pos_list->cbegin(), pos_list->cend(), false, pos_list);
  };
};
}
}
