// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <atomic>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <memory>
#include <sstream>

#include "helper/types.h"
#include "helper/checked_cast.h"

#include "storage/AbstractHashTable.h"
#include "storage/AbstractTable.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace storage {

template<class MAP, class KEY> class HashTableView;

// Group of value_ids as key to an unordered map
typedef std::vector<value_id_t> aggregate_key_t;
// Group of hashed values as key to an unordered map
typedef std::vector<size_t> join_key_t;

// Single Value ID as key to unordered map
typedef value_id_t aggregate_single_key_t;
// Single Hashed Value
typedef size_t join_single_key_t;

size_t hash_value(const c_atable_ptr_t &source, const size_t &f, const ValueId &vid);

template <typename HashResult>
inline typename HashResult::value_type extract(const c_atable_ptr_t &table,
    const size_t &field,
    const ValueId &vid);

template <>
inline typename join_key_t::value_type extract<join_key_t>(const c_atable_ptr_t &table,
    const size_t &field,
    const ValueId &vid) {
  return hash_value(table, field, vid);
}

template <>
inline typename aggregate_key_t::value_type extract<aggregate_key_t>(const c_atable_ptr_t &table,
    const size_t &field,
    const ValueId &vid) {
  return vid.valueId;
}

// Helper Functions for Single Values
template<typename HashResult>
inline HashResult extractSingle(const c_atable_ptr_t &table,
    const size_t &field,
    const ValueId &vid);

template <>
inline join_single_key_t extractSingle<join_single_key_t>(const c_atable_ptr_t &table,
    const size_t &field,
    const ValueId &vid) {
  return hash_value(table, field, vid);
}

template <>
inline aggregate_single_key_t extractSingle<aggregate_single_key_t>(const c_atable_ptr_t &table,
    const size_t &field,
    const ValueId &vid) {
  return vid.valueId;
}

// hash function for aggregate_key_t
template<class T>
class GroupKeyHash {
public:
  size_t operator()(const T &key) const {
    static auto hasher = std::hash<value_id_t>();

    std::size_t seed = 0;
    for (size_t i = 0, key_size = key.size();
         i < key_size; ++i) {
      // compare boost hash_combine
      seed ^= hasher(key[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }

  static T getGroupKey(const c_atable_ptr_t &table,
                       const field_list_t &columns,
                       const size_t fieldCount,
                       const pos_t row) {
    ValueIdList value_list = table->copyValueIds(row, &columns);
    T key;
    for (size_t i = 0, key_size = fieldCount; i < key_size; i++)
      key.push_back(extract<T>(table, columns[i], value_list[i]));
    return key;
  }
};

// Simple Hash Function for single values
template<class T>
class SingleGroupKeyHash {
public:
  size_t operator()(const T &key) const {
    static auto hasher = std::hash<value_id_t>();
    return hasher(key);
  }

  static T getGroupKey(const c_atable_ptr_t &table,
                       const field_list_t &columns,
                       const size_t fieldCount,
                       const pos_t row){
    return extractSingle<T>(table, columns[0], table->getValueId(columns[0], row));
  }
};

// Multi Keys
typedef std::unordered_multimap<aggregate_key_t, pos_t, GroupKeyHash<aggregate_key_t> > aggregate_hash_map_t;
typedef std::unordered_multimap<join_key_t, pos_t, GroupKeyHash<join_key_t> > join_hash_map_t;

// Single Keys
typedef std::unordered_multimap<aggregate_single_key_t, pos_t, SingleGroupKeyHash<aggregate_single_key_t> > aggregate_single_hash_map_t;
typedef std::unordered_multimap<join_single_key_t, pos_t, SingleGroupKeyHash<join_single_key_t> > join_single_hash_map_t;

/// HashTable based on a map; key specifies the key for the given map
template<class MAP, class KEY> class HashTable;
typedef HashTable<aggregate_hash_map_t, aggregate_key_t> AggregateHashTable;
typedef HashTable<join_hash_map_t, join_key_t> JoinHashTable;

// HashTables for single values
typedef HashTable<aggregate_single_hash_map_t, aggregate_single_key_t> SingleAggregateHashTable;
typedef HashTable<join_single_hash_map_t, join_single_key_t> SingleJoinHashTable;

/// Uses valueIds of specified columns as key for an unordered_multimap
template <class MAP, class KEY>
class HashTable : public AbstractHashTable, public std::enable_shared_from_this<HashTable<MAP, KEY> > {
public:
  typedef KEY key_t;
  typedef MAP map_t;
  typedef typename map_t::const_iterator map_const_iterator_t;
  typedef decltype(std::declval<const map_t>().equal_range(key_t())) map_const_range_t;

protected:

  // Underlaying storage type
  map_t _map;

  // Reference to the table
  c_atable_ptr_t _table;
  
  // Fields in map
  const field_list_t _fields;

  // Cached num keys
  mutable std::atomic<uint64_t> _numKeys;
  mutable std::atomic<bool> _dirty;

private:

  // populates map with values
  inline void populate_map(size_t row_offset = 0) {
    _dirty = true;
    size_t fieldSize = _fields.size();
    size_t tableSize = _table->size();
    for (pos_t row = 0; row < tableSize; ++row) {
      key_t key = MAP::hasher::getGroupKey(_table, _fields, fieldSize, row);
      _map.insert(typename map_t::value_type(key, row + row_offset));
    }
  }

  pos_list_t constructPositions(const map_const_range_t &range) const {
    return constructPositions(range.first, range.second);
  }

  pos_list_t constructPositions(const map_const_iterator_t &begin,  const map_const_iterator_t &end) const {
    pos_list_t positions(std::distance(begin, end));
    // decltype(*range.first) returns the type of iterator elements
    std::transform(begin, end, positions.begin(), [&](decltype(*begin)& value) {
      return value.second;
    });
    return positions;
  }

public:
  HashTable() {}

  // create a new HashTable based on a number of HashTables
  explicit HashTable(const std::vector<std::shared_ptr<const AbstractHashTable> >& hashTables) {
    _dirty = true;
    for (auto & nextElement: hashTables) {
      const auto& ht = checked_pointer_cast<const HashTable<MAP, KEY>>(nextElement);
      _map.insert(ht->getMapBegin(), ht->getMapEnd());
    }
  }

  // Hash given table's columns directly into the new HashTable
  // row_offset is used if t is a TableRangeView, so that the HashTable can build the pos_lists based on the row numbers of the original table
  HashTable(c_atable_ptr_t t, const field_list_t &f, size_t row_offset = 0)
    : _table(t), _fields(f), _numKeys(0), _dirty(true) {
    populate_map(row_offset);
  }

  virtual ~HashTable() {}

  std::string stats() const {
    std::stringstream s;
    s << "Load Factor " << _map.load_factor() << " / ";
    s << "Max Load Factor " << _map.max_load_factor() << " / ";
    s << "Bucket Count " << _map.bucket_count();
    return s.str();
  }

  std::shared_ptr<HashTableView<MAP, KEY> > view(size_t first, size_t last) const {
    return std::make_shared<HashTableView<MAP, KEY>>(this->shared_from_this(), first, last);
  }

  /// Returns the number of key value pairs of underlying hash map structure.
  virtual size_t size() const {
    return _map.size();
  }

  /// Get positions for values given in the table by row and columns.
  virtual pos_list_t get(const c_atable_ptr_t &table,
                         const field_list_t &columns,
                         const pos_t row) const {
    pos_list_t pos_list;
    key_t key = MAP::hasher::getGroupKey(table, columns, columns.size(), row);
    auto range = _map.equal_range(key);
    return constructPositions(range);
  }

  /// Get const interators to underlying map's begin or end.
  map_const_iterator_t getMapBegin() const {
    return _map.begin();
  }

  map_const_iterator_t getMapEnd() const {
    return _map.end();
  }

  c_atable_ptr_t getTable() const {
    return _table;
  }

  field_list_t getFields() const {
    return _fields;
  }

  size_t getFieldCount() const {
    return _fields.size();
  }

  map_t &getMap() {
    return _map;
  }

  virtual pos_list_t get(const key_t &key) const {
    auto range = _map.equal_range(key);
    return constructPositions(range);
  }

  uint64_t numKeys() const {
    if (_dirty) {
      uint64_t result = 0;
      for (map_const_iterator_t it1 = _map.begin(), it2 = it1, end = _map.end(); it1 != end; it1 = it2) {
        for (; (it2 != end) && (it1->first == it2->first); ++it2) {}
        ++result;
      }

      _numKeys = result;
      _dirty = false;
    }
    return _numKeys;
  }
};

/// Maps table cells' hashed values of arbitrary columns to their rows.
/// This subclass maps only a range of key value pairs of its underlying
/// HashTable for an easy splitting
template <class MAP, class KEY>
class HashTableView : public AbstractHashTable {
public:
  typedef HashTable<MAP, KEY> hash_table_t;

protected:
  std::shared_ptr<const hash_table_t> _hashTable;
  typename hash_table_t::map_const_iterator_t _begin;
  typename hash_table_t::map_const_iterator_t _end;
  typedef KEY key_t;

  mutable std::atomic<uint64_t> _numKeys;
  mutable std::atomic<bool> _dirty;

public:
  /// Given a HashTable and a range, only the n-ths key value pairs of the
  /// given HashTable corresponding to the range will be mapped by this view.
  HashTableView(const std::shared_ptr<const hash_table_t>& tab,
                const size_t start,
                const size_t end) :
  _hashTable(tab), _begin(_hashTable->getMapBegin()), _end(_hashTable->getMapBegin()), _numKeys(0), _dirty(true) {

    _begin = advance(start);
    _end = advance(end);
  }


  typename hash_table_t::map_const_iterator_t advance(size_t val) {
    size_t counter = 0;
    if (val == 0)
      return _hashTable->getMapBegin();

    for (typename hash_table_t::map_const_iterator_t it1 = _hashTable->getMapBegin(), it2 = it1, end = _hashTable->getMapEnd(); 
      it1 != end; it1 = it2) {

      // Skip to next key-value pair
      for (; (it2 != end) && (it1->first == it2->first); ++it2);
      if (++counter == val)
        return it2;
    }
    //if (counter == val)
      return _hashTable->getMapEnd();
    //throw std::runtime_error("Could not advance to position");
  }


  virtual ~HashTableView() {}

  /// Returns the number of key value pairs of underlying hash map structure.
  size_t size() const {
    return std::distance(_begin, _end);
  }

  /// Get positions for values in the table cells of given row and columns.
  /// TODO: check whether copy to new unordered_map and search via equal_range is faster
  virtual pos_list_t get(
    const c_atable_ptr_t &table,
    const field_list_t &columns,
    const pos_t row) const {

    pos_list_t pos_list;
    // produce key
    key_t key = MAP::hasher::getGroupKey(table, columns, columns.size(), row);

    for (typename hash_table_t::map_const_iterator_t it = _begin; it != _end; ++it) {
      if (it->first == key) {
        pos_list.push_back(it->second);
      }
    }
    return pos_list;
  }

  pos_list_t get(key_t key) const {
    pos_list_t pos_list;

    for (typename hash_table_t::map_const_iterator_t it = _begin; it != _end; ++it) {
      if (it->first == key) {
        pos_list.push_back(it->second);
      }
    }
    return pos_list;
  }


  /// Get const interators to underlying map's begin or end.
  typename hash_table_t::map_const_iterator_t getMapBegin() const {
    return _begin;
  }
  typename hash_table_t::map_const_iterator_t getMapEnd() const {
    return _end;
  }

  atable_ptr_t getHashTable() const {
    return _hashTable;
  }

  field_list_t getFields() const {
    return _hashTable->getFields();
  }

  size_t getFieldCount() const {
    return _hashTable->getFieldCount();
  }

  c_atable_ptr_t getTable() const {
    return _hashTable->getTable();
  }

  uint64_t numKeys() const {
    if (_dirty) {
      uint64_t result = 0;
      for (typename hash_table_t::map_const_iterator_t it1 = _begin, it2 = it1, end = _end; it1 != end; it1 = it2) {
        for (; (it2 != end) && (it1->first == it2->first); ++it2);
        ++result;
      }
      return _numKeys = result;
    }
    return _numKeys;
  }
};

} } // namespace hyrise::storage

