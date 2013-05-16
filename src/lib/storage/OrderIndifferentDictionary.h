// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ORDERINDIFFERENTDICTIONARY_H_
#define SRC_LIB_STORAGE_ORDERINDIFFERENTDICTIONARY_H_

#include <storage/storage_types.h>
#include <storage/AbstractDictionary.h>
#include <storage/BaseDictionary.h>
#include <storage/DictionaryIterator.h>
#include <storage/csb_tree.h>
#include <storage/AbstractAllocatedDictionary.h>

#include <exception>
#include <vector>
#include <map>
#include <memory>
#include <limits.h>

#include <map>

// FIXME should be aware of allocator
template <typename T>
class OrderIndifferentDictionaryIterator : public BaseIterator<T> {
public:

  typedef typename std::map<T, value_id_t>::iterator iterator_t;
  iterator_t _it;

  OrderIndifferentDictionaryIterator(): _it(nullptr) {}

  explicit OrderIndifferentDictionaryIterator(iterator_t it): _it(it) {
    //cout << "order indifferent dict it copy constr" << std::endl;
  }

  void increment() {
    ++_it;
  }

  bool equal(BaseIterator<T> *other) const {
    return _it == ((OrderIndifferentDictionaryIterator<T> *) other)->_it;
  }

  T &dereference() const {
    return (T &)(*_it).first;
  }

  value_id_t getValueId() const {
    return (*_it).second;
  }

  virtual BaseIterator<T> *clone() {
    //std::cout << "t2" << std::endl;
    return new OrderIndifferentDictionaryIterator<T>(*this);
  }

};


/*
  An unordered dictionary directly returns the offset of the value
  inside the value list. An auxillary structure is kept to allow
  easy sorted iteratos and logarithmic finds.
*/
template < typename T,
         class Strategy = MallocStrategy,
         template <typename K, typename S> class Allocator = StrategizedAllocator
         >
class OrderIndifferentDictionary : public BaseAllocatedDictionary < T,
  Strategy,
  Allocator,
    OrderIndifferentDictionary<T, Strategy, Allocator> > {

  typedef Strategy strategy_type;
  typedef std::map<T, value_id_t, std::less<T>, Allocator<std::pair<T, value_id_t>, Strategy> > index_type;
  typedef std::vector<T, Allocator<T, Strategy> > vector_type;

  // This is the main index
  index_type _index;


public:

  // This is used for the values
  vector_type _value_list;

  explicit OrderIndifferentDictionary(size_t s = 0) {
    if (s > 0) {
      reserve(s);
    }
  }

  virtual ~OrderIndifferentDictionary() {
  }

  void shrink() {
    _value_list.shrink_to_fit();
  }

  bool isOrdered() {
    return false;
  }

  size_t size() {
    return _value_list.size();
  }

  std::shared_ptr<AbstractDictionary> copy() {
    // FIXME
    auto res = std::make_shared<OrderIndifferentDictionary<T, Strategy, Allocator>>();
    res->_index = _index;
    res->_value_list = _value_list;
    return res;
  }

  void reserve(size_t s) {
    _value_list.reserve(s);
  }

  virtual T getValueForValueId(value_id_t value_id) {
    return _value_list[value_id];
  }

  virtual value_id_t addValue(T value) {
    _value_list.push_back(value);
    addValueToIndex(value);
    return _value_list.size() - 1;
  }

  inline void addValueToIndex(T value) {
    _index.insert(std::pair<T, value_id_t>(value, _value_list.size() - 1));
  }

  virtual bool isValueIdValid(value_id_t value_id) {
    return value_id < _value_list.size();
  }

  virtual bool valueExists(const T &v) const {
    return _index.count(v) == 1;
  }

  /*
   *  Returns the value id given a value
   *
   * Unfortunately its not possible to not perform a better search, since
   * the value lookup is always linear
   *
   * @note complexity is O(n)
   *
   * @param value the value
   */
  virtual value_id_t getValueIdForValue(const T &value) const {
    if (_index.count(value) > 0) {
      return _index.at(value);
    }

    throw std::runtime_error("Value not found");
  }

  value_id_t getValueIdForValueSmaller(T other) {
    throw std::runtime_error("This cannot be called since value ids have no ordered meaning");
  }

  value_id_t getValueIdForValueGreater(T other) {
    throw std::runtime_error("This cannot be calles since value ids have no ordered meaning");
  }

  const T getSmallestValue() {
    throw std::runtime_error("This cannot be called in an unordered dictionary");
  }

  const T getGreatestValue() {
    throw std::runtime_error("This cannot be called in an unordered dictionary");
  }

  typedef DictionaryIterator<T> iterator;

  // returns an iterator pointing to the beginning of the tree
  iterator begin() {
    return iterator(new OrderIndifferentDictionaryIterator<T>(_index.begin()));
  }

  // returns an empty iterator that marks the end of the tree
  iterator end() {
    return iterator(new OrderIndifferentDictionaryIterator<T>(_index.end()));
  }

  void build_index() {

    for (size_t i = 0; i < _value_list.size(); ++i) {
      addValueToIndex(_value_list[i]);
    }
  }

};

#endif  // SRC_LIB_STORAGE_ORDERINDIFFERENTDICTIONARY_H_

