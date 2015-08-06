#pragma once

#include <memory>
#include "helper/not_implemented.h"
#include "storage/BaseDictionary.h"
#include "storage/DictionaryIterator.h"
#include "tbb/concurrent_vector.h"
#include "tbb/concurrent_unordered_map.h"

namespace hyrise {
namespace storage {

template <typename T>
class ConcurrentUnorderedDictionaryIterator : public BaseIterator<T> {
  typedef ConcurrentUnorderedDictionaryIterator<T> iter_type;
  typedef typename std::map<T, value_id_t>::iterator iter_t;
  iter_t _it;

 public:
  ConcurrentUnorderedDictionaryIterator(iter_t it) : _it(it) {}

  void increment() { _it++; }

  bool equal(const std::shared_ptr<BaseIterator<T>>& other) const {
    return (_it == std::static_pointer_cast<iter_type>(other)->_it);
  }

  T& dereference() const { return (T&)_it->first; }

  value_id_t getValueId() const { return _it->second; }
};

template <typename T>
class ConcurrentUnorderedDictionary : public BaseDictionary<T> {
 public:
  explicit ConcurrentUnorderedDictionary(const size_t s = 0) : _values(s), _size(s) {}

  ConcurrentUnorderedDictionary(const std::shared_ptr<std::vector<T>>& value_list) {
    _writeLock.lock();
    for (auto value : *value_list) {
      auto inserted = _values.push_back(value);
      auto result = std::distance(_values.begin(), inserted);
      _index.insert({value, result});
      ++_size;
    }
    _writeLock.unlock();
  }


  // Semantics differ from other dictionaries: adding the same value twice yields
  // the same valueId. This is due to multiple threads pushing back the same
  // value and/or testing for its existance via valueExists
  // and the lag with inserting the resulting position into _index_unordered,
  // where the first writer wins.
  virtual value_id_t addValue(T value) override {
    auto inserted = _values.push_back(value);
    auto result = std::distance(_values.begin(), inserted);
    _writeLock.lock();
    auto r = _index.insert({value, result});
    _writeLock.unlock();
    ++_size;
    return r.first->second;
  }



  virtual T getValueForValueId(value_id_t value_id) override { return _values.at(value_id); }

  virtual value_id_t getValueIdForValue(const T& value) const override {
#ifdef EXPENSIVE_ASSERTIONS
    auto end = _values.end();
    assert(std::find(_values.begin(), end, value) != end);
#endif
    return _index.find(value)->second;
  }

  virtual value_id_t findValueIdForValue(const T& value) const {
    auto it = _index.find(value);
    if (it != _index.end()) {
      return it->second;
    } else {
      return std::numeric_limits<value_id_t>::max();
    }
  }

  virtual bool isValueIdValid(value_id_t value_id) override { return value_id < _size; }

  virtual bool valueExists(const T& value) const override { return _index.find(value) != _index.end(); }
  virtual const T getSmallestValue() { return *std::min(_values.begin(), _values.end()); }
  virtual const T getGreatestValue() { return *std::max(_values.begin(), _values.end()); }
  virtual void reserve(std::size_t s) override { _values.grow_to_at_least(s); }
  virtual std::size_t size() override { return _size; }

  virtual bool isOrdered() override { return false; }
  virtual std::shared_ptr<AbstractDictionary> copy() override {
    auto d = std::make_shared<ConcurrentUnorderedDictionary<T>>(size());
    d->_values = _values;
    d->_index = _index;
    return d;
  }
  virtual std::shared_ptr<AbstractDictionary> copy_empty() override {
    return std::make_shared<ConcurrentUnorderedDictionary<T>>();
  }
  virtual void shrink() {}

  typedef DictionaryIterator<T> iterator;

  // Unsafe method, calling this assumes no concurrent callers
  virtual iterator begin() override {
    _index_sorted.clear();
    _index_sorted.insert(_index.begin(), _index.end());
    return iterator(std::make_shared<ConcurrentUnorderedDictionaryIterator<T>>(_index_sorted.begin()));
  }
  virtual iterator end() override {
    return iterator(std::make_shared<ConcurrentUnorderedDictionaryIterator<T>>(_index_sorted.end()));
  }

  virtual value_id_t getLowerBoundValueIdForValue(T other) {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionary, getValueIdForValueSmaller());
  }

  virtual value_id_t getUpperBoundValueIdForValue(T other) {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionary, getValueIdForValueGreater());
  }

  void setValueId(T value, value_id_t valueId) {
    // WARNING: do not use this, it's only for table recovery
    if (_size <= valueId) {
      reserve(valueId + 1);
      _size = valueId + 1;
    }
    _values[valueId] = value;
    _index.insert({value, valueId});
  }



  // These iterators are used to directly access the underlying vector for checkpointing.
  typedef typename tbb::concurrent_vector<T>::iterator unsorted_iterator;
  // returns an iterator pointing to the beginning of the tree
  unsorted_iterator unsorted_begin() { return _values.begin(); }

  // returns an empty iterator that marks the end of the tree
  unsorted_iterator unsorted_end() { return _values.end(); }

 private:
  tbb::concurrent_vector<T> _values;
  tbb::concurrent_unordered_map<T, value_id_t> _index;
  std::map<T, value_id_t> _index_sorted;
  size_t _size = 0;
  std::mutex _writeLock;
};
}
}  // namespace hyrise::storage
