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
#include "storage/OrderPreservingDictionary.h"
#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/CompoundValueIdKeyBuilder.h"
#include "storage/meta_storage.h"
#include "storage/ConcurrentFixedLengthVector.h"
#include <storage/Store.h>

#include <memory>

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace hyrise {
namespace storage {

// An inverted index for the main that is created once
// on the main, manually or during a merge.
// The index is readonly.

/*
  This class is currently used for two purposes: A single , templated index and the compound Index.
  This leads to a couple of problems, which need to be solved. It seems better to have two distinct classes,
  esp. since the compound version could be implemented more efficiently, that currently.
*/

template <typename T>
class GroupkeyIndex : public AbstractIndex {
 private:
  typedef std::map<T, pos_list_t> inverted_index_mutable_t;
  typedef OrderPreservingDictionary<T> dict_t;
  typedef std::vector<pos_t> groupkey_offsets_t;
  typedef std::vector<pos_t> groupkey_postings_t;
  typedef std::shared_ptr<dict_t> dict_ptr_t;

  typedef std::pair<typename pos_list_t::const_iterator, typename pos_list_t::const_iterator> range_t;

  dict_ptr_t _dictionary;
  std::vector<T> _dictionary_values;
  const std::vector<field_t> _columns;
  groupkey_offsets_t _offsets;
  groupkey_postings_t _postings;

 public:
  virtual ~GroupkeyIndex() {}

  std::vector<pos_t>::iterator offsetsBegin() { return _offsets.begin(); }

  std::vector<pos_t>::iterator offsetsEnd() { return _offsets.end(); }

  std::vector<pos_t>::iterator postingsBegin() { return _postings.begin(); }

  std::vector<pos_t>::iterator postingsEnd() { return _postings.end(); }

  void shrink() { throw std::runtime_error("Shrink not supported for GroupkeyIndex"); }

  void write_lock() {}

  void unlock() {}

  void print() {
    auto postingsIterator = postingsBegin();
    uint64_t i = 0;
    for (auto offsetsIterator = offsetsBegin(); offsetsIterator != offsetsEnd(); ++offsetsIterator) {
      std::cout << i << ": " << *offsetsIterator << ": ";
      if (offsetsIterator + 1 != offsetsEnd()) {
        size_t p = *(offsetsIterator + 1) - *offsetsIterator;
        for (size_t y = 0; y < p; ++y) {
          std::cout << *postingsIterator << ", ";
          postingsIterator++;
        }
      }
      std::cout << std::endl;
      i++;
    }
  }

  std::shared_ptr<AbstractIndex> recreateIndex(const c_atable_ptr_t& in, field_t column) {
    return std::make_shared<GroupkeyIndex<T>>(in, column, true, _id);
  }

  std::shared_ptr<AbstractIndex> recreateIndexMergeDict(size_t column,
                                                        std::shared_ptr<Store> store,
                                                        std::shared_ptr<AbstractDictionary>& newDictReturn,
                                                        std::vector<value_id_t>& vidMappingMain,
                                                        std::vector<value_id_t>& vidMappingDelta) override {
    atable_ptr_t oldMain = store->getMainTable();
    atable_ptr_t oldDelta = store->getDeltaTable();

    size_t mainTableSize = oldMain->size();
    auto deltaVector = std::dynamic_pointer_cast<storage::ConcurrentFixedLengthVector<value_id_t>>(
        oldDelta->getAttributeVectors(column).at(0).attribute_vector);

    auto mainDictionary =
        std::dynamic_pointer_cast<storage::OrderPreservingDictionary<T>>(oldMain->dictionaryAt(column, 0, 0));
    auto deltaDictionary =
        std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<T>>(oldDelta->dictionaryAt(column, 0, 0));

    auto newDict = std::make_shared<OrderPreservingDictionary<T>>(deltaDictionary->size());

    auto it = mainDictionary->begin();
    auto itDelta = deltaDictionary->begin();
    auto deltaEnd = deltaDictionary->end();

    groupkey_postings_t newPostings;
    groupkey_offsets_t newOffsets;

    size_t mainDictSize = mainDictionary->size();

    T mainDictValue;

    ValueId vid;

    uint64_t count, c = 0, m = 0, n = 0;
    auto postingIterator = postingsBegin();
    auto offsetIterator = offsetsBegin();

    // mapping of values to positions in deltaVector necessary to be able to build help structure for merging (_Vd)
    // efficiently
    std::map<T, std::vector<size_t>> deltaValueMap;
    for (size_t j = 0; j < deltaVector->size(); ++j) {
      deltaValueMap[deltaDictionary->getValueForValueId(deltaVector->getRef(0, j))].push_back(j);
    }

    while (itDelta != deltaEnd || m != mainDictSize) {
      if (m != mainDictSize)
        mainDictValue = mainDictionary->getValueForValueId(m);

      bool processM = (itDelta == deltaEnd || (m != mainDictSize && mainDictValue <= *itDelta));
      bool processD = (m == mainDictSize || (*itDelta <= mainDictValue));

      newOffsets.push_back(c);

      if (processM) {
        vid.valueId = newDict->addValue(mainDictValue);

        vidMappingMain.push_back(n - m);
        ++m;

        count = *(offsetIterator + 1) - *offsetIterator;
        c += count;
        for (size_t j = 0; j < count; ++j) {
          newPostings.push_back(*postingIterator);
          ++postingIterator;
        }
        ++offsetIterator;
      }

      if (processD) {
        if (!processM)
          vid.valueId = newDict->addValue(*itDelta);

        count = 0;
        for (auto it = deltaValueMap[*itDelta].cbegin(); it != deltaValueMap[*itDelta].cend(); ++it) {
          ++count;
          newPostings.push_back(*it + mainTableSize);
          vidMappingDelta[*it] = n;
        }

        c += count;
        ++itDelta;
      }
      ++n;
    }

    newOffsets.push_back(c);

    newDictReturn = newDict;
    return std::make_shared<GroupkeyIndex<T>>(column, newDict, newOffsets, newPostings, _id);
  }

  explicit GroupkeyIndex(field_t column,
                         dict_ptr_t dictionary,
                         groupkey_offsets_t offsets,
                         groupkey_postings_t postings,
                         std::string id = "volatile_groupkey")
      : AbstractIndex(id), _dictionary(dictionary), _columns({column}), _offsets(offsets), _postings(postings) {};

  explicit GroupkeyIndex(const c_atable_ptr_t& in,
                         field_t column,
                         bool create = true,
                         std::string id = "volatile_groupkey",
                         bool will_recover_from_archive = false)
      : AbstractIndex(id), _columns({column}) {
    if (in != nullptr && create && !will_recover_from_archive) {
      // save pointer to dictionary
      _dictionary = std::static_pointer_cast<dict_t>(in->dictionaryAt(column));

      // create mutable index
      inverted_index_mutable_t _index;
      for (size_t row = 0; row < in->size(); ++row) {
        T tmp = in->getValue<T>(column, row);
        typename inverted_index_mutable_t::iterator find = _index.find(tmp);
        if (find == _index.end()) {
          pos_list_t pos;
          pos.push_back(row);
          _index[tmp] = pos;
        } else {
          find->second.push_back(row);
        }
      }

      // create readonly index
      _offsets.resize(_dictionary->size() + 1);
      _postings.reserve(in->size());

      for (auto& it : _index) {
        // set offset
        auto value_id = _dictionary->getValueIdForValue(it.first);
        _offsets[value_id] = _postings.size();

        // copy positions into postings
        std::copy(it.second.begin(), it.second.end(), std::back_inserter(_postings));
      }

      // set last offset
      _offsets[_dictionary->size()] = _postings.size();
    } else if (in != nullptr && !create) {
      _dictionary = std::static_pointer_cast<dict_t>(in->dictionaryAt(column));
    } else {
      throw std::runtime_error("Nullptr as input for GroupkeyIndex constructor.");
    }
  }

  // CompoundKey Version.
  explicit GroupkeyIndex(const c_atable_ptr_t& in,
                         std::vector<field_t>& columns,
                         bool create = true,
                         const std::string id = "volatile_groupkey",
                         bool will_recover_from_archive = false)
      : AbstractIndex(id), _columns(columns) {
    _dictionary = std::make_shared<dict_t>();
    if (in != nullptr && create && !will_recover_from_archive) {
      // _dictionary->setValueVector(_dictionary_values);

      std::vector<value_id_t> dictionary_sizes;
      for (auto column : columns) {
        if (dictionary_sizes.size() <= column)
          dictionary_sizes.resize(column + 1);
        dictionary_sizes[column] = in->dictionaryAt(column)->size();
      }

      // create mutable index
      inverted_index_mutable_t _index;

      for (size_t row = 0; row < in->size(); ++row) {
        CompoundValueIdKeyBuilder builder;
        for (auto column : columns) {
          builder.add(in->getValueId(column, row).valueId, dictionary_sizes[column]);
        }

        T compound_key = builder.get();
        typename inverted_index_mutable_t::iterator find = _index.find(compound_key);
        if (find == _index.end()) {
          pos_list_t pos;
          pos.push_back(row);
          _index[compound_key] = pos;
        } else {
          find->second.push_back(row);
        }
      }

      for (auto&& it : _index) {
        _dictionary->addValue(it.first);
      }

      // create readonly index
      _offsets.resize(_dictionary->size() + 1);
      _postings.reserve(in->size());

      for (auto& it : _index) {
        // set offset
        auto value_id = _dictionary->getValueIdForValue(it.first);
        _offsets[value_id] = _postings.size();

        // copy positions into postings
        std::copy(it.second.begin(), it.second.end(), std::back_inserter(_postings));
      }

      // set last offset
      _offsets[_dictionary->size()] = _postings.size();
    } else if (in != nullptr && !create) {
    } else {
      throw std::runtime_error("Nullptr as input for GroupkeyIndex constructor.");
    }
  }


  /**
   * returns a list of positions where key was found.
   */
  PositionRange getPositionsForKey(T key) {
    if (_dictionary->size() == 0) {
      throw std::runtime_error("dict=0");
    }

    auto value_id = _dictionary->findValueIdForValue(key);
    if (value_id != std::numeric_limits<value_id_t>::max()) {
      auto start = _offsets[value_id];
      auto end = _offsets[value_id + 1];
      return PositionRange(_postings.begin() + start, _postings.begin() + end, true);
    } else {
      // empty result
      return PositionRange(_postings.begin(), _postings.begin(), true);
    }
  }

  PositionRange getPositionsForKeyLT(T key) {
    auto value_id = _dictionary->getLowerBoundValueIdForValue(key);
    const bool value_exists = value_id < _dictionary->size();

    if (value_exists) {
      auto end = _offsets[value_id];
      return PositionRange(_postings.begin(), _postings.begin() + end, false);
    } else {
      // all
      return PositionRange(_postings.begin(), _postings.end(), false);
    }
  }

  PositionRange getPositionsForKeyLTE(T key) {
    auto value_id = _dictionary->getUpperBoundValueIdForValue(key);
    const bool value_exists = value_id < _dictionary->size();

    if (value_exists) {
      auto end = _offsets[value_id];
      return PositionRange(_postings.begin(), _postings.begin() + end, false);
    } else {
      // all
      return PositionRange(_postings.begin(), _postings.end(), false);
    }
  }

  PositionRange getPositionsForKeyGT(T key) {
    auto value_id = _dictionary->getUpperBoundValueIdForValue(key);
    const bool value_exists = value_id < _dictionary->size();

    if (value_exists) {
      auto start = _offsets[value_id];
      return PositionRange(_postings.begin() + start, _postings.end(), false);
    } else {
      // empty
      return PositionRange(_postings.end(), _postings.end(), false);
    }
  }

  PositionRange getPositionsForKeyGTE(T key) {
    auto value_id = _dictionary->getLowerBoundValueIdForValue(key);
    const bool value_exists = value_id < _dictionary->size();

    if (value_exists) {
      auto start = _offsets[value_id];
      return PositionRange(_postings.begin() + start, _postings.end(), false);
    } else {
      // empty
      return PositionRange(_postings.end(), _postings.end(), false);
    }
  }

  // returns range [a,b]
  PositionRange getPositionsForKeyBetween(T a, T b) {
    // return range [a,b]
    if (a > b)
      std::swap(a, b);

    auto value_id_a = _dictionary->getLowerBoundValueIdForValue(a);
    auto value_id_b = _dictionary->getUpperBoundValueIdForValue(b);
    const bool value_a_exists = value_id_a < _dictionary->size();
    const bool value_b_exists = value_id_b < _dictionary->size();

    if (value_a_exists && value_b_exists) {
      auto start = _offsets[value_id_a];
      auto end = _offsets[value_id_b];
      return PositionRange(_postings.begin() + start, _postings.begin() + end, false);
    } else if (value_a_exists) {
      auto start = _offsets[value_id_a];
      return PositionRange(_postings.begin() + start, _postings.end(), false);
    } else if (value_b_exists) {
      auto end = _offsets[value_id_b];
      return PositionRange(_postings.begin(), _postings.begin() + end, false);
    } else {
      // empty
      return PositionRange(_postings.end(), _postings.end(), false);
    }
  }

  template <class Archive>
  void serialize(Archive& ar) {
    _dictionary->swapValues(_dictionary_values);
    ar(_offsets, _postings, _dictionary_values);
    _dictionary->swapValues(_dictionary_values);
  }

  const std::vector<field_t>& getColumns() const { return _columns; }
};
}
}
