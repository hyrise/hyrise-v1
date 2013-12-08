// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <memory>

#include "helper/checked_cast.h"
#include "storage/BaseDictionary.h"
#include "storage/BaseIterator.h"
#include "storage/DictionaryIterator.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace storage {

template <typename T>
class OrderPreservingDictionaryIterator;

template <typename T>
class OrderPreservingDictionary : public BaseDictionary<T> {
public:
  typedef std::vector<T> vector_type;
  typedef std::shared_ptr<vector_type> shared_vector_type;
private:
  shared_vector_type _values;

protected:

  // This constructor is only used for copying purposes
  explicit OrderPreservingDictionary(vector_type values) {
      _values = std::make_shared<vector_type>();
      std::copy(values.begin(), values.end(), values->begin());
  }

public:

  OrderPreservingDictionary() {
    _values = std::make_shared<vector_type>();
  }
  
  explicit OrderPreservingDictionary(size_t size) {
    _values = std::make_shared<vector_type>();
    _values->reserve(size);
  }

  virtual ~OrderPreservingDictionary() {}

  void shrink() {
    _values->shrink_to_fit();
  }

  /**
   * Return value of given value id
   *
   * @param value the value id
   * @return value of the value id
   */
  value_id_t addValue(T value) {
#ifdef EXPENSIVE_ASSERTIONS
    if ((_values->size() > 0) && (value <= _values->back()))
      throw std::runtime_error("Can't insert value smaller or equal to last value");
#endif
    _values->push_back(value);
    return _values->size() - 1;
  }

  /**
   * Return value of given value id
   *
   * @param value the value id
   * @return value of the value
   */
  T getValueForValueId(value_id_t value_id) {
#ifdef EXPENSIVE_ASSERTIONS
    if (value_id >= _values->size())
      throw std::out_of_range("Trying to access value_id larger than available values");
#endif
    return (*_values)[value_id];
  }
      
  value_id_t getValueIdForValue(const T &value) const {
    auto binary_search = std::lower_bound(_values->begin(), _values->end(), value);
    size_t index = binary_search - _values->begin();
    return index;
  }

  value_id_t getValueIdForValueSmaller(T other) {
    auto binary_search = std::lower_bound(_values->begin(), _values->end(), other);
    size_t index = binary_search - _values->begin();
    
    assert(index > 0);
    return index - 1;
  }

  value_id_t getValueIdForValueGreater(T other) {
    auto binary_search = std::upper_bound(_values->begin(), _values->end(), other);
    size_t index = binary_search - _values->begin();
    
    return index;
  }

  const T getSmallestValue() {
    assert(_values->size() > 0);
    return (*_values)[0];
  }
    
  const T getGreatestValue() {
    assert(_values->size() > 0);
    return (*_values)[_values->size() - 1];
  }

  bool isValueIdValid(value_id_t value_id) {
    return value_id < _values->size();
  }

  bool valueExists(const T &value) const {
    return binary_search(_values->begin(), _values->end(), value);
  }

  void reserve(size_t size) {
    _values->reserve(size);
  }
  
  size_t size() {
    return _values->size();
  }

  std::shared_ptr<AbstractDictionary> copy() {
    throw std::runtime_error("Dictionaries cannot be copied");
  }

  std::shared_ptr<AbstractDictionary> copy_empty() {
    return std::make_shared<OrderPreservingDictionary<T> >();
  }
  
  bool isOrdered() {
    return true;
  }

  typedef DictionaryIterator<T> iterator;

  iterator begin() {
    return iterator(std::make_shared<OrderPreservingDictionaryIterator<T>>(_values, 0));
  }

  iterator end() {
    return iterator(std::make_shared<OrderPreservingDictionaryIterator<T>>(_values, _values->size()));
  }

};


/*
 * This is the Iterator class that is used by the dictionary. It has a
 * shared public interface to be able to differentiate between the
 * different dictionary types and to abstract from data types.
 */
template <typename T>
class OrderPreservingDictionaryIterator : public BaseIterator<T> {

  typedef OrderPreservingDictionary<T> dictionary_type;
  typedef typename dictionary_type::shared_vector_type vector_type;

public:
  const vector_type& _values;
  size_t _index;

  explicit OrderPreservingDictionaryIterator(const vector_type& values): _values(values), _index(0) {}

  OrderPreservingDictionaryIterator(const vector_type& values, size_t index): _values(values), _index(index) {}

  virtual ~OrderPreservingDictionaryIterator() { }

  void increment() {
    ++_index;
  }

  bool equal(const std::shared_ptr<BaseIterator<T>>& other) const {
    return
        _values.get() == std::dynamic_pointer_cast<OrderPreservingDictionaryIterator<T>>(other)->_values.get() &&
        _index  == std::dynamic_pointer_cast<OrderPreservingDictionaryIterator<T>>(other)->_index;
  }

  T &dereference() const {
    return (*_values)[_index];
  }

  value_id_t getValueId() const {
    return _index;
  }

};

} } // namespace hyrise::storage

