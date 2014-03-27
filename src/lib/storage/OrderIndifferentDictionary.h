// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <exception>
#include <vector>
#include <map>
#include <memory>
#include <limits.h>
#include <mutex>

#include <map>

#include "storage/storage_types.h"
#include "storage/BaseDictionary.h"
#include "storage/DictionaryIterator.h"

namespace hyrise {
namespace storage {

// FIXME should be aware of allocator
template <typename T>
class OrderIndifferentDictionaryIterator : public BaseIterator<T> {
 public:
  typedef typename std::map<T, value_id_t>::iterator iterator_t;
  iterator_t _it;

  OrderIndifferentDictionaryIterator() : _it(nullptr) {}

  explicit OrderIndifferentDictionaryIterator(iterator_t it) : _it(it) {
    // cout << "order indifferent dict it copy constr" << std::endl;
  }

  void increment() { ++_it; }

  bool equal(const std::shared_ptr<BaseIterator<T>>& other) const {
    return _it == std::dynamic_pointer_cast<OrderIndifferentDictionaryIterator<T>>(other)->_it;
  }

  T& dereference() const { return (T&)(*_it).first; }

  value_id_t getValueId() const { return (*_it).second; }
};


/*
  An unordered dictionary directly returns the offset of the value
  inside the value list. An auxillary structure is kept to allow
  easy sorted iteratos and logarithmic finds.
*/
template <typename T>
class OrderIndifferentDictionary : public BaseDictionary<T> {
  typedef std::map<T, value_id_t> index_type;
  typedef std::vector<T> vector_type;

  // This is the main index
  index_type _index;
  std::mutex _writeLock;

 public:
  // This is used for the values
  vector_type _value_list;

  explicit OrderIndifferentDictionary(size_t s = 0) {
    if (s > 0) {
      reserve(s);
    }
  }

  OrderIndifferentDictionary(const std::shared_ptr<vector_type>& value_list) {
    // first, copy the value list
    _value_list = *(value_list.get());

    // second, build tree index on top of this
    size_t s = _value_list.size();
    for (size_t i = 0; i < s; ++i)
      _index.insert(std::pair<T, value_id_t>(_value_list[i], i));
  }

  virtual ~OrderIndifferentDictionary() {}

  void shrink() { _value_list.shrink_to_fit(); }

  bool isOrdered() { return false; }

  size_t size() { return _value_list.size(); }

  std::shared_ptr<AbstractDictionary> copy() {
    // FIXME
    auto res = std::make_shared<OrderIndifferentDictionary<T>>();
    res->_index = _index;
    res->_value_list = _value_list;
    return res;
  }

  std::shared_ptr<AbstractDictionary> copy_empty() { return std::make_shared<OrderIndifferentDictionary<T>>(); }

  void reserve(size_t s) { _value_list.reserve(s); }

  virtual T getValueForValueId(value_id_t value_id) {
#ifdef EXPENSIVE_ASSERTIONS
    if (value_id >= _value_list.size()) {
      throw std::out_of_range("value id out of range");
    }
#endif
    return _value_list[value_id];
  }

  virtual value_id_t addValue(T value) {
    _writeLock.lock();
    _value_list.push_back(value);
    addValueToIndex(value);
    size_t new_size = _value_list.size() - 1;
    _writeLock.unlock();
    return new_size;
  }

  inline void addValueToIndex(T value) { _index.insert(std::pair<T, value_id_t>(value, _value_list.size() - 1)); }

  virtual bool isValueIdValid(value_id_t value_id) { return value_id < _value_list.size(); }

  virtual bool valueExists(const T& v) const { return _index.count(v) == 1; }

  virtual value_id_t getValueIdForValue(const T& value) const {
    if (_index.count(value) == 0) {
      throw std::out_of_range("Value not found");
    }
    return _index.at(value);
  }

  virtual value_id_t findValueIdForValue(const T& value) const {
    typename index_type::const_iterator it = _index.find(value);
    if (it != _index.end()) {
      return it->second;
    } else {
      return std::numeric_limits<value_id_t>::max();
    }
  }

  value_id_t getLowerBoundValueIdForValue(T other) {
    throw std::runtime_error("This cannot be called since value ids have no ordered meaning");
  }

  value_id_t getUpperBoundValueIdForValue(T other) {
    throw std::runtime_error("This cannot be calles since value ids have no ordered meaning");
  }

  const T getSmallestValue() { throw std::runtime_error("This cannot be called in an unordered dictionary"); }

  const T getGreatestValue() { throw std::runtime_error("This cannot be called in an unordered dictionary"); }

  typedef DictionaryIterator<T> iterator;

  // returns an iterator pointing to the beginning of the tree
  iterator begin() { return iterator(std::make_shared<OrderIndifferentDictionaryIterator<T>>(_index.begin())); }

  // returns an empty iterator that marks the end of the tree
  iterator end() { return iterator(std::make_shared<OrderIndifferentDictionaryIterator<T>>(_index.end())); }

  void build_index() {

    for (size_t i = 0; i < _value_list.size(); ++i) {
      addValueToIndex(_value_list[i]);
    }
  }

  virtual void setValue(T value, value_id_t value_id) {
    // this is used when restoring the database. You probably don't want to use this during regular
    // operations
    if (_value_list.size() <= value_id)
      _value_list.resize(value_id + 1);
    _value_list[value_id] = value;
    _index.insert(std::pair<T, value_id_t>(value, value_id));
  }
};
}
}  // namespace hyrise::storage
