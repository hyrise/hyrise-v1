#pragma once
// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <iterator>

#include <tbb/concurrent_hash_map.h>


namespace hyrise {
namespace helper {

template<typename T>
class ConcurrentSparseVectorIterator;

template<typename T>
class ConcurrentSparseVector {

  using map_t = tbb::concurrent_hash_map<size_t, T>;

  // This is the data to keep the TX
  map_t _data;

  size_t _size = 0;

  T _defaultValue;


public:

  template<typename V>
  struct Proxy {
    size_t _index = 0;
    V& _data;

    Proxy(V & d, size_t i) : _index(i), _data(d) {
    }

    // Implicit conversion
    operator const T () const {
      return _data.get(_index);
    }

    Proxy& operator=(const T& val) {
      _data.set(_index, val);
      return *this;
    }

  };


  using value_type = T;
  using iterator = ConcurrentSparseVectorIterator<T>;
  using const_iterator = const ConcurrentSparseVectorIterator<T>;
  
  ConcurrentSparseVector() {}

  ConcurrentSparseVector(size_t size, T defaultValue) : _size(size), _defaultValue(defaultValue)  {}

  T get(size_t index) const {
    typename map_t::const_accessor c;
    if (_data.find(c, index)) 
      return c->second;
    else
      return _defaultValue;
  }

  void resize(size_t count) {
    if (count < _size) {
      throw std::runtime_error("Concurrent sparse vector can only grow");
    }

    _size = count;
  }

  /**
   * Resize is not atomic with non-default value
   */
  void resize(size_t count, T val) {
    auto oldSize = size();
    resize(count);
    if (val != _defaultValue) {
      for(size_t i=oldSize; i < count; ++i) {
	set(i, val);
      }
    }

  }

  void set(size_t index, const T& val) {
    typename map_t::accessor c;
    if (_data.insert(c, index)) {
      if (val == _defaultValue) {
	_data.erase(c);
	return;
      }
    }
    c->second = val;
  }

  bool cmpxchg(size_t index, const T& oldVal, const T& newVal) {
    typename map_t::accessor c;
    
    // If the index was not used until now
    // In addition we use the accessor as a write lock o the key
    if (_data.insert(c, index)) {
      if (oldVal == _defaultValue) {
	c->second = newVal;
	return true;
      } else {
	// Clear the newly inserted key
	_data.erase(c);
	return false;
      }
    } else {

      // If the position is used, check if the values are correct and
      // remove the newval if it is the default value
      if ( c->second == oldVal) {

	if (newVal == _defaultValue) {
	  _data.erase(c);
	} else {
	  c->second = newVal;
	}
	return true;
      } else {
	return false;
      }
    }
  }

  size_t size() const { 
    return _size;
  }

  void push_back(const T& val) {
    auto oldSize = _size++;
    typename map_t::accessor c;
    if (_data.insert(c, oldSize))
      c->second = val;
    else
      throw std::runtime_error("push_back() out of bounds");
  }

  const Proxy<const ConcurrentSparseVector<T> > operator[] (const size_t index) const {
    return Proxy<const ConcurrentSparseVector<T> >(*this, index);
  }
  
  Proxy<ConcurrentSparseVector<T> > operator[] (const size_t index) {
    return Proxy<ConcurrentSparseVector<T> >(*this, index);
  }


  iterator begin() {
    return iterator(this);
  }

  iterator end() {
    return iterator(this, size());
  }

  const_iterator begin() const {
    return iterator(this);
  }

  const_iterator end() const {
    return iterator(this, size());
  }

  const_iterator cbegin() const {
    return iterator(this);
  }

  const_iterator cend() const {
    return iterator(this, size());
  }


  
  

};

template<typename T>
class ConcurrentSparseVectorIterator : std::iterator<std::forward_iterator_tag, T> {

  size_t _pos;
  ConcurrentSparseVector<T>* _vector;

public:

  using iterator = ConcurrentSparseVectorIterator<T>;
  

  explicit ConcurrentSparseVectorIterator(ConcurrentSparseVector<T>* v, size_t p = 0) : _pos(p), _vector(v) {
  }


  bool operator == (const ConcurrentSparseVectorIterator<T>& other) const {
    return _vector == other._vector && _pos == other._pos;
  }

  bool operator != (const ConcurrentSparseVectorIterator<T>& other) const {
    return !(operator==(other));
  }

  T operator* () const {
    return _vector->get(_pos);
  }

  T operator-> () const {
    return _vector->get(_pos);
  }

  iterator& operator++() {
    ++_pos;
    return *this;
  }

  iterator operator++(int) {
    auto res = *this;
    ++_pos;
    return res;
  }


};

}}
