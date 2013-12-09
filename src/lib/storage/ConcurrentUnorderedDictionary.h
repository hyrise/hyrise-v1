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

  void increment() {
    _it++;
  }

  bool equal(const std::shared_ptr<BaseIterator<T>>& other) const {
    return (_it == std::static_pointer_cast<iter_type>(other)->_it);
  }

  T &dereference() const {
    return (T&) _it->first;
  }

  value_id_t getValueId() const {
    return _it->second;
  }
};

template <typename T>
class ConcurrentUnorderedDictionary : public BaseDictionary<T> {
 public:
  explicit ConcurrentUnorderedDictionary(const size_t s=0) : _values(s) { }

  // Semantics differ from other dictionaries: adding the same value twice yields
  // the same valueId. This is due to multiple threads pushing back the same
  // value and/or testing for its existance via valueExists
  // and the lag with inserting the resulting position into _index_unordered,
  // where the first writer wins.
  virtual value_id_t addValue(T value) override {
    auto inserted = _values.push_back(value);
    auto result = std::distance(_values.begin(), inserted);
    auto r = _index_unordered.insert({value, result});
    return r.first->second;
  }

  virtual T getValueForValueId(value_id_t value_id) override {
    return _values.at(value_id);
  }

  virtual value_id_t getValueIdForValue(const T& value) const override {
#ifdef EXPENSIVE_ASSERTIONS
    auto end = _values.end();
    assert(std::find(_values.begin(), end, value) != end);
#endif
    return _index_unordered.at(value);
  }

  virtual bool isValueIdValid(value_id_t value_id) override {
    return value_id < _values.size();
  }

  virtual bool valueExists(const T& value) const override {
    return _index_unordered.count(value) >= 1;
  }
  virtual const T getSmallestValue() {
    return *std::min(_values.begin(), _values.end());
  }
  virtual const T getGreatestValue() {
    return *std::max(_values.begin(), _values.end());
  }
  virtual void reserve(std::size_t s) override {
    _values.grow_to_at_least(s);
  }
  virtual std::size_t size() override {
    return _values.size();
  }
  virtual bool isOrdered() override {
    return false;
  }
  virtual std::shared_ptr<AbstractDictionary> copy() override {
    auto d = std::make_shared<ConcurrentUnorderedDictionary<T>>(size());
    d->_values = _values;
    d->_index_unordered = _index_unordered;
    return d;
  }
  virtual std::shared_ptr<AbstractDictionary> copy_empty() override {
    return std::make_shared<ConcurrentUnorderedDictionary<T>>();
  }
  virtual void shrink() {}

  typedef DictionaryIterator<T> iterator;

  // Unsafe method, calling this assumes no concurrent callers
  virtual iterator begin() override {
    _index.clear();
    _index.insert(_index_unordered.begin(), _index_unordered.end());
    return iterator(std::make_shared<ConcurrentUnorderedDictionaryIterator<T> >(_index.begin()));
  }
  virtual iterator end() override {
    return iterator(std::make_shared<ConcurrentUnorderedDictionaryIterator<T> >(_index.end()));
  }
  virtual value_id_t getValueIdForValueSmaller(T other) { NOT_IMPLEMENTED }
  virtual value_id_t getValueIdForValueGreater(T other) { NOT_IMPLEMENTED }
 private:
  tbb::concurrent_unordered_map<T, value_id_t> _index_unordered; // a potentially laggy set
  tbb::concurrent_vector<T> _values;
  std::map<T, value_id_t> _index;
};

} } // namespace hyrise::storage

