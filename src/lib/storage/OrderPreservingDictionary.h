// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cstring>

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

  T dereference() const {
    return (*_values)[_index];
  }

  value_id_t getValueId() const {
    return _index;
  }

};


/// String Dictionary Specialization
class OrderPreservingDictionaryIteratorString;

template <>
class OrderPreservingDictionary<std::string> : public BaseDictionary<std::string> {

  using char_trait = std::char_traits<char>;
  using char_ptr_t = char_trait::char_type*;

  // Offset into the char memory
  std::vector<char_trait::off_type> _offset;
  
  char_ptr_t _data = nullptr;
  char_ptr_t _end = nullptr;
  char_ptr_t _endOfStorage = nullptr;


  /*
   * Returns a const char pointer to the string
   */
  char_ptr_t get_c(size_t index) const {
    return _data + _offset[index];
  }

  size_t strlen(size_t index) const {
    if ((index + 1) < _offset.size()) {
      return _offset[index+1] - _offset[index];
    } else {
      return _end - (_data + _offset[index]);
    }
  }


  void reserve_raw(size_t bytes) {
    auto oldEnd = _end - _data;
    _data = static_cast<char_ptr_t>(realloc(_data, bytes * sizeof(char_trait::char_type)));
    _end = _data + oldEnd;
    _endOfStorage = _data + bytes;
  }

  size_t allocated() const {
    return _endOfStorage - _data;
  }


public:

  OrderPreservingDictionary() {
  }
  
  explicit OrderPreservingDictionary(size_t size) {
  }

  virtual ~OrderPreservingDictionary() {
    free(_data);
  }

  void shrink() {
  }

  /**
   * Return value of given value id
   *
   * @param value the value id
   * @return value of the value id
   */
  value_id_t addValue(std::string value) {
#ifdef EXPENSIVE_ASSERTIONS
//    if ((_values->size() > 0) && (value <= _values->back()))
//      throw std::runtime_error("Can't insert value smaller or equal to last value");
#endif
    if ((_end + value.size()) > _endOfStorage) {
      reserve_raw(allocated() * 2);
    }

    memcpy(_end, value.c_str(), value.size());
    _offset.emplace_back(_end - _data);
    _end = _end + value.size();
    return _offset.back();
  }


  /*
   * String transformation
   */
  std::string get_s(size_t index) const {
    return std::string(get_c(index), strlen(index));
  }
  
  /**
   * Return value of given value id
   *
   * @param value the value id
   * @return value of the value
   */
  std::string getValueForValueId(value_id_t value_id) {
#ifdef EXPENSIVE_ASSERTIONS
    // if (value_id >= _values->size())
    //   throw std::out_of_range("Trying to access value_id larger than available values");
#endif
    std::string tmp(get_c(value_id), strlen(value_id)); 
    return std::move(tmp);
  }
      
  value_id_t getValueIdForValue(const std::string &value) const {
    auto binary_search = std::lower_bound(std::begin(_offset), std::end(_offset), value, [&](const char_trait::off_type& o, const std::string& v){
        return std::strncmp(get_c(o), v.c_str(), strlen(o)) < 0;
      });
    return *binary_search;
  }

  value_id_t getValueIdForValueSmaller(std::string other) {
    // auto binary_search = std::lower_bound(_values->begin(), _values->end(), other);
    // size_t index = binary_search - _values->begin();
    
    // assert(index > 0);
    // return index - 1;
    return 0;
  }

  value_id_t getValueIdForValueGreater(std::string other) {
    // auto binary_search = std::upper_bound(_values->begin(), _values->end(), other);
    // size_t index = binary_search - _values->begin();
    
    // return index;
    return 0;
  }

  const std::string getSmallestValue() {
    return std::string(get_c(0), strlen(0));
  }
    
  const std::string getGreatestValue() {
    return std::string(get_c(_offset.back()), strlen(_offset.back()));
  }

  bool isValueIdValid(value_id_t value_id) {
    return value_id < _offset.size();
  }

  bool valueExists(const std::string &value) const {
    auto res = std::lower_bound(std::begin(_offset), std::end(_offset), value, [&](const char_trait::off_type& o, const std::string& v){
        return std::strncmp(get_c(o), v.c_str(), strlen(o)) < 0;
      });
    return res != std::end(_offset);
  }

  void reserve(size_t size) {
  }
  
  size_t size() {
    return _offset.size();
  }

  std::shared_ptr<AbstractDictionary> copy() {
    throw std::runtime_error("Dictionaries cannot be copied");
  }

  std::shared_ptr<AbstractDictionary> copy_empty() {
    return std::make_shared<OrderPreservingDictionary<std::string> >();
  }
  
  bool isOrdered() {
    return true;
  }

  typedef DictionaryIterator<std::string> iterator;

  iterator begin() {
    return iterator(std::make_shared<OrderPreservingDictionaryIteratorString >(this, 0));
  }

  iterator end() {
    
  }

};

class OrderPreservingDictionaryIteratorString : public BaseIterator<std::string> {

  using dictionary_type =  OrderPreservingDictionary<std::string>;
  using base_type = OrderPreservingDictionaryIteratorString;

  dictionary_type *_data = nullptr;
  size_t _index = 0;

public:

  explicit OrderPreservingDictionaryIteratorString(dictionary_type *values): _data(values) {}

  OrderPreservingDictionaryIteratorString(dictionary_type *values, size_t index): _data(values), _index(index) {}

  virtual ~OrderPreservingDictionaryIteratorString() { }

  void increment() {
    ++_index;
  }

  bool equal(const std::shared_ptr<BaseIterator<std::string>>& other) const {
    return 
      _data == std::dynamic_pointer_cast<base_type>(other)->_data &&
      _index  == std::dynamic_pointer_cast<base_type>(other)->_index;
  }

  std::string dereference() const {
    return std::move(_data->get_s(_index));
  }

  value_id_t getValueId() const {
    return _index;
  }

};













} } // namespace hyrise::storage

