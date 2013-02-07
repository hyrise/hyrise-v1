// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ORDERPRESERVINGDICTIONARY_H_
#define SRC_LIB_STORAGE_ORDERPRESERVINGDICTIONARY_H_

#include <assert.h>
#include <iostream>
#include <memory>

#include <storage/storage_types.h>
#include <storage/AbstractDictionary.h>
#include <storage/BaseDictionary.h>
#include <storage/DictionaryIterator.h>
#include <storage/AbstractAllocatedDictionary.h>

#include <storage/BaseIterator.h>
#include <storage/DictionaryIterator.h>

template <typename T, class Strategy, template <typename A, typename B> class Allocator>
class OrderPreservingDictionaryIterator;

template < typename T,
         class Strategy = MallocStrategy,
         template <typename K, typename S> class Allocator = StrategizedAllocator
         >
class OrderPreservingDictionary : public BaseAllocatedDictionary < T,
  Strategy,
  Allocator,
    OrderPreservingDictionary<T, Strategy, Allocator> > {

public:
  typedef std::vector<T, Allocator<T, Strategy> > vector_type;
  typedef std::shared_ptr<vector_type> shared_vector_type;

private:

  shared_vector_type _values;

protected:

  // This constructor is only used for copying purposes
  explicit OrderPreservingDictionary(vector_type values);

public:

  OrderPreservingDictionary();
  explicit OrderPreservingDictionary(size_t size);

  virtual ~OrderPreservingDictionary() {
  }

  void shrink() {
    _values->shrink_to_fit();
  }

  value_id_t addValue(T value);

  T getValueForValueId(value_id_t value_id);
  value_id_t getValueIdForValue(const T &value) const;

  value_id_t getValueIdForValueSmaller(T other);
  value_id_t getValueIdForValueGreater(T other);

  const T getSmallestValue();
  const T getGreatestValue();

  bool isValueIdValid(value_id_t value_id);

  bool valueExists(const T &value) const;

  void reserve(size_t size);
  size_t size();

  std::shared_ptr<AbstractDictionary> copy();
  
  bool isOrdered();

  typedef DictionaryIterator<T> iterator;

  iterator begin() {
    return iterator(new OrderPreservingDictionaryIterator<T, Strategy, Allocator>(_values, 0));
  }

  iterator end() {
    return iterator(new OrderPreservingDictionaryIterator<T, Strategy, Allocator>(_values, _values->size()));
  }

};


template <typename T, class Strategy, template <typename K, typename S> class Allocator>
OrderPreservingDictionary<T, Strategy, Allocator>::OrderPreservingDictionary() {
  _values = std::make_shared<vector_type>();
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
OrderPreservingDictionary<T, Strategy, Allocator>::OrderPreservingDictionary(size_t size) {
  _values = std::make_shared<vector_type>();
  _values->reserve(size);
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
OrderPreservingDictionary<T, Strategy, Allocator>::OrderPreservingDictionary(vector_type values) {
  _values = std::make_shared<vector_type>();
  std::copy(values.begin(), values.end(), values->begin());
}

/**
 * Return value of given value id
 *
 * @param value the value id
 * @return value of the value id
 */
template <typename T, class Strategy, template <typename K, typename S> class Allocator>
value_id_t OrderPreservingDictionary<T, Strategy, Allocator>::addValue(T value) {
#ifdef EXPENSIVE_ASSERTIONS
  if ((_values->size() > 0) && (value <= _values->back()))
    throw std::runtime_error("Can't insert value smaller than last value");
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
template <typename T, class Strategy, template <typename K, typename S> class Allocator>
T OrderPreservingDictionary<T, Strategy, Allocator>::getValueForValueId(value_id_t value_id) {
#ifdef EXPENSIVE_ASSERTIONS
  if (value_id >= _values->size())
    throw std::runtime_error("Trying to access value_id larger than available values");
#endif
  return (*_values)[value_id];
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
value_id_t OrderPreservingDictionary<T, Strategy, Allocator>::getValueIdForValueSmaller(T other) {
  auto binary_search = lower_bound(_values->begin(), _values->end(), other);
  size_t index = binary_search - _values->begin();

  assert(index > 0);
  return index - 1;
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
value_id_t OrderPreservingDictionary<T, Strategy, Allocator>::getValueIdForValueGreater(T other) {
  auto binary_search = upper_bound(_values->begin(), _values->end(), other);
  size_t index = binary_search - _values->begin();

  return index;
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
const T OrderPreservingDictionary<T, Strategy, Allocator>::getSmallestValue() {
  assert(_values->size() > 0);

  return (*_values)[0];
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
const T OrderPreservingDictionary<T, Strategy, Allocator>::getGreatestValue() {
  assert(_values->size() > 0);

  return (*_values)[_values->size() - 1];
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
value_id_t OrderPreservingDictionary<T, Strategy, Allocator>::getValueIdForValue(const T &value) const {
  // needs refactoring
  auto binary_search = lower_bound(_values->begin(), _values->end(), value);
  size_t index = binary_search - _values->begin();
  return index;
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
bool OrderPreservingDictionary<T, Strategy, Allocator>::isValueIdValid(value_id_t value_id) {
  return value_id < _values->size();
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
bool OrderPreservingDictionary<T, Strategy, Allocator>::valueExists(const T &value) const {
  return binary_search(_values->begin(), _values->end(), value);
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
void OrderPreservingDictionary<T, Strategy, Allocator>::reserve(size_t size) {
  _values->reserve(size);
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
size_t OrderPreservingDictionary<T, Strategy, Allocator>::size() {
  return _values->size();
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
std::shared_ptr<AbstractDictionary> OrderPreservingDictionary<T, Strategy, Allocator>::copy() {
  throw std::runtime_error("Dictionaries cannot be copied");
}

template <typename T, class Strategy, template <typename K, typename S> class Allocator>
bool OrderPreservingDictionary<T, Strategy, Allocator>::isOrdered() {
  return true;
}

/*
 * This is the Iterator class that is used by the dictionary. It has a
 * shared public interface to be able to differentiate between the
 * different dictionary types and to abstract from data types.
 *
 */
template <typename T, class Strategy = MallocStrategy, template<typename A, typename B> class Allocator = StrategizedAllocator>
class OrderPreservingDictionaryIterator : public BaseIterator<T> {

  typedef OrderPreservingDictionary<T, Strategy, Allocator> dictionary_type;
  typedef typename dictionary_type::shared_vector_type vector_type;

public:


  vector_type _values;
  size_t _index;

  explicit OrderPreservingDictionaryIterator(vector_type values): _values(values), _index(0) {}

  OrderPreservingDictionaryIterator(vector_type values, size_t index): _values(values), _index(index) {}

  virtual ~OrderPreservingDictionaryIterator() { }

  void increment() {
    ++_index;
  }

  bool equal(BaseIterator<T> *other) const {
    return _values.get() == ((OrderPreservingDictionaryIterator<T, Strategy, Allocator> *) other)->_values.get() &&
           _index  == ((OrderPreservingDictionaryIterator<T, Strategy, Allocator> *) other)->_index;
  }

  T &dereference() const {
    return (*_values)[_index];
  }

  value_id_t getValueId() const {
    return _index;
  }

  virtual BaseIterator<T> *clone() {
    return new OrderPreservingDictionaryIterator<T, Strategy, Allocator>(*this);
  }

};

#endif  // SRC_LIB_STORAGE_ORDERPRESERVINGDICTIONARY_H_
