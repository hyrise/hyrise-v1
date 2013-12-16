// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>

#include "helper/types.h"

#include "storage/storage_types.h"
#include "storage/AbstractIndex.h"
#include "storage/AbstractTable.h"

#include <unordered_map>
#include <memory>

namespace hyrise {
namespace storage {

template<typename T>
class InvertedIndex : public AbstractIndex {
private:
  //using inverted_index_t = std::map<T, pos_list_t>;
  using inverted_index_t = std::unordered_map<T, pos_list_t>;
  inverted_index_t _index;

public:
  virtual ~InvertedIndex() {};

  void shrink() {
    for (auto & e : _index) {
      e.second.shrink_to_fit();
    }
  }

  explicit InvertedIndex(const c_atable_ptr_t& in, field_t column) {
    if (in != nullptr) {
      for (size_t row = 0; row < in->size(); ++row) {
        T tmp = in->getValue<T>(column, row);
        typename inverted_index_t::iterator find = _index.find(tmp);
        if (find == _index.end()) {
          pos_list_t pos;
          pos.push_back(row);
          _index[tmp] = pos;
        } else {
          find->second.push_back(row);
        }
      }
    }
  };

  /**
   * returns a list of positions where key was found.
   */
  pos_list_t getPositionsForKey(T key) {
    typename inverted_index_t::iterator it = _index.find(key);
    if (it != _index.end()) {
      return it->second;
    } else {
      pos_list_t empty;
      return empty;
    }
  };

  
  bool exists(T key) const {
    return _index.count(key) > 0;
  }

  const pos_list_t& getPositionsForKeyRef(T key) {
    const auto& it = _index.find(key);
    return it->second;
  };

};

} } // namespace hyrise::storage

