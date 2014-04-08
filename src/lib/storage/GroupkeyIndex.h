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

  std::string _id;
  dict_ptr_t _dictionary;
  std::vector<T> _dictionary_values;
  const std::vector<field_t> _columns;
  groupkey_offsets_t _offsets;
  groupkey_postings_t _postings;

 public:
  virtual ~GroupkeyIndex() {}

  std::vector<pos_t>::iterator offsetsBegin() {
    return _offsets.begin();
  }

  std::vector<pos_t>::iterator offsetsEnd() {
    return _offsets.end();
  }

  std::vector<pos_t>::iterator postingsBegin() {
    return _postings.begin();
  }

  std::vector<pos_t>::iterator postingsEnd() {
    return _postings.end();
  }

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

  bool recreateIndexMergeDict(size_t i, atable_ptr_t oldMain, atable_ptr_t oldDelta, atable_ptr_t newMain, std::shared_ptr<hyrise::storage::AbstractDictionary> newDictResult, std::vector<std::vector<value_id_t>> &x, value_id_t* VdColumn) {
    // std::cout << "############## BEFORE INDEX ##############" << std::endl;
    // print();
    size_t mainTableSize = oldMain->size();
    std::shared_ptr<hyrise::storage::ConcurrentFixedLengthVector<value_id_t>> deltaVector = std::dynamic_pointer_cast<storage::ConcurrentFixedLengthVector<value_id_t>>(oldDelta->getAttributeVectors(i).at(0).attribute_vector);

    std::shared_ptr<hyrise::storage::OrderPreservingDictionary<T>> mainDictionary = std::dynamic_pointer_cast<storage::OrderPreservingDictionary<T>>(oldMain->dictionaryAt(i, 0, 0));
    std::shared_ptr<hyrise::storage::ConcurrentUnorderedDictionary<T>> deltaDictionary = std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<T>>(oldDelta->dictionaryAt(i, 0, 0));

    auto newDict = std::make_shared<OrderPreservingDictionary<T>>(deltaDictionary->size());

    auto it = mainDictionary->begin();
    auto itDelta = deltaDictionary->begin();
    auto deltaEnd = deltaDictionary->end();

    std::vector<pos_t> newPostings;
    std::vector<pos_t> newOffsets;

    ValueId vid;

    uint64_t count, c = 0, m = 0, n = 0;
    std::vector<value_id_t> xColumn;
    auto postingIterator = postingsBegin();
    auto offsetIterator = offsetsBegin();

    while (itDelta != deltaDictionary->end() || m != mainDictionary->size()) {
      // TODO: store result of mainDictionary->getValueForValueId(m) in variable
      bool processM = (m != mainDictionary->size() && (mainDictionary->getValueForValueId(m) <= *itDelta || itDelta == deltaDictionary->end()));
      bool processD = (m == mainDictionary->size() || *itDelta <= mainDictionary->getValueForValueId(m));

      newOffsets.push_back(c);

      if (processM) {
        vid.valueId = newDict->addValue(mainDictionary->getValueForValueId(m));

        xColumn.push_back(n - m);
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
        if (!newDict->valueExists(*itDelta))
          vid.valueId = newDict->addValue(*itDelta);

        count = 0;
        // TODO: optimize
        for (size_t j = 0; j < deltaVector->size(); ++j) {
          if (deltaDictionary->getValueForValueId(deltaVector->getRef(0, j)) == *itDelta) {
            ++count;
            newPostings.push_back(j + mainTableSize);
            VdColumn[j] = n;
          }
        }
        c += count;
        ++itDelta;
      }
      ++n;
    }

    newOffsets.push_back(c);

    newMain->setDictionaryAt(newDict, i);
    x.push_back(xColumn);

    _offsets = newOffsets;
    _postings = newPostings;

    // std::cout << "############## AFTER INDEX ##############" << std::endl;
    // print();

    newDictResult = newDict;

    return true;
  }

  explicit GroupkeyIndex(const c_atable_ptr_t& in,
                         field_t column,
                         bool create = true,
                         std::string id = "volatile_groupkey",
                         bool will_recover_from_archive = false)
      : _id(id), _columns({column}) {
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
      : _id(id), _columns(columns) {
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
