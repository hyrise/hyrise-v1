#pragma once
// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <algorithm>
#include <atomic>
#include <iostream>
#include <numeric>
#include <vector>
#include <sstream>
#include <iterator>


namespace hyrise {
namespace helper {


namespace detail {
template<typename T, typename U, typename F>
auto foldLeft(const T& values, U initial, F func ) -> U {
  return std::accumulate(values.begin(), values.end(), initial, func);
}
}


template<typename V>
class SparseVectorIterator;

/**
 * The general idea of the sparse vector is that it contains a list of values
 * where only a small fraction of the values is different from a predefined
 * default value. In addition, the vector will be initalized with a relatively
 * large amount of items based on the default value.
 */
template<typename T, size_t BucketSize=6*1024*1024>
class SparseVector {


public:

  using value_type = T;
  using reference_type = value_type&;

  // Block types
  using block_type = std::vector<value_type>;
  using block_type_ptr = block_type*;

  using atomic_block_type_ptr = std::atomic<block_type_ptr>;

  using iterator = SparseVectorIterator<SparseVector<T, BucketSize> >;
  using const_iterator = SparseVectorIterator<const SparseVector<T, BucketSize> >;

  // Overlay
  using mapping_type = std::vector<atomic_block_type_ptr>;
  constexpr static size_t per_mapping_size = BucketSize / sizeof(T);

  static size_t blocksToAllocate(size_t size) {
    return (size / per_mapping_size) + 1;
  }


protected:

  size_t _allocSize = 0;

  // This is the default value of the vector, cannot be changed
  value_type _defaultValue;

  // THis is the list of materialzed mappings that we keep to manage the data
  mapping_type _mappings;

  // This block manages all appends to the data structure
  block_type _append;

  size_t offset(size_t pos) const {
    return pos / per_mapping_size;
  }

  size_t inpos(size_t pos) const {
    return pos % per_mapping_size;
  }

  // Used to implement copy-swap-idiom
  friend void swap(SparseVector& left, SparseVector& right) {
    std::swap(left._defaultValue, right._defaultValue);
    std::swap(left._mappings, right._mappings);
    std::swap(left._append, right._append);
    std::swap(left._allocSize, right._allocSize);
  }

public:

  std::string debug() const {
    std::stringstream out;
    out << "SparseVector: " << this << std::endl;
    out << "Per Block: " << per_mapping_size << std::endl;
    out << "Size: " << size() << std::endl;
    out << "Allocated Blocks: " << detail::foldLeft(_mappings, 0u, [](size_t m, const atomic_block_type_ptr& x){
        const auto& v = x.load();
        return m + (v == nullptr ? 0 : 1);
      }) << "/" << _mappings.size() << std::endl;
    out << "Empty Blocks: " << detail::foldLeft(_mappings, 0u, [](size_t m, const atomic_block_type_ptr& x){
        const auto& v = x.load();
        return m + (v == nullptr ? 1 : 0);
      }) << "/" << _mappings.size() << std::endl;
    return out.str();
  }

  template<typename V>
  struct Proxy {
    V& _vector;
    size_t _index;

    Proxy(V& v, size_t index) : _vector(v), _index(index) {}

    operator const T () const {
      return _vector.get(_index);
    }

    Proxy& operator=(const T& val) {
      _vector.set(_index, val);
      return *this;
    }

  };


  // This is the default constructor that is used to implement all the
  // functionality, all other ctors forward here
  explicit SparseVector(size_t size, T defaultValue = T()) : 
    _allocSize(size), _defaultValue(defaultValue), _mappings(blocksToAllocate(size)) {
  }


  SparseVector(const SparseVector& other) : SparseVector(other._allocSize, _defaultValue) {
    _append = other._append;
    for(size_t i=0; i < other._mappings.size(); ++i) {
      auto tmp = other._mappings[i].load();
      if (tmp != nullptr) {
        _mappings[i] = new block_type(*tmp);
      }
    }
  }


  SparseVector() : SparseVector(0) {
  }

  ~SparseVector() {
    for(const auto& x : _mappings) {
      if (x.load() != nullptr) {
        delete x.load();
      }
    }
  }

  SparseVector& operator=(SparseVector other) {
    swap(*this, other);
    return *this;
  }

  
  /**
   * Returns the size of the container
   */
  size_t size() const {
    return _allocSize + _append.size();
  }

  value_type get(size_t pos) const {
    if (pos < _allocSize) {
      const auto off = offset(pos);
      const auto relPos = inpos(pos);
      const auto& x = _mappings[off].load(std::memory_order_relaxed);

      if (x == nullptr)
        return _defaultValue;
      else
        return (*x)[relPos];
    } else {
      return _append[pos - _allocSize];
    }
  }

  const Proxy<const SparseVector> operator[] (size_t index) const {
    return Proxy<const SparseVector>(*this, index);
  }

  Proxy<SparseVector> operator[] (size_t index) {
    return Proxy<SparseVector>(*this, index);
  }

  /*
   * Setting a value, even though it is the default value will result in
   * allocating the memory, as we assume that once set it will happen again
   * (temporal localtiy) To make this threadsafe we have to do perform the
   * following:
   *
   * 1) First we load the pointer to the materialized vector atomically, if
   *    this is non null we can safely write as nobody will ever touch this
   *    piece of memory again
   * 2) If the pointer is null we create a new vector of the reserved size and
   *    set the value, finally, we try to comp exchange the allocaed value with
   *    the nullptr.
   */
  void set(size_t pos, value_type value) {
    if (pos < _allocSize) {
      const auto off = offset(pos);
      auto x = _mappings[off].load(std::memory_order_relaxed);
      if (x != nullptr) {
        // First case
        (*x)[inpos(pos)] = value;
      } else {
        // Second case, initialize with default value
        block_type_ptr data = new block_type(per_mapping_size, _defaultValue);
        (*data)[inpos(pos)] = value;

        // Perform the replace by compare xchange, if the comp exchange fails,
        // recurse this method to retry
        if (!_mappings[off].compare_exchange_weak(x, data, std::memory_order_release, std::memory_order_relaxed)) {
          delete data;
          set(pos, value);
        }
      }
    } else {
      _append[pos - _allocSize] = value;
    }
  }

  // Resizes are always materialized
  void resize(size_t count, value_type val) {
    if (count <=  size()) {
      throw std::runtime_error("Resize has to increase the storage!");
    }
    _append.resize(count - _allocSize, val);
  }

  
  // Compare and swap helper for the vector
  bool cmpxchg(size_t pos, value_type oldVal, value_type newVal) {
    if (pos < _allocSize) {
      const auto off = offset(pos);
      auto x = _mappings[off].load(std::memory_order_relaxed);
      if (x != nullptr) {
        // First case
        std::atomic<value_type> atom(((*x)[inpos(pos)]));
        return atom.compare_exchange_weak(oldVal, newVal, std::memory_order_release, std::memory_order_relaxed);
      } else {
        // Second case, initialize with default value
        block_type_ptr data = new block_type(per_mapping_size, _defaultValue);
        (*data)[inpos(pos)] = newVal;

        // Perform the replace by compare xchange, if the comp exchange fails,
        // recurse this method to retry
        if (!_mappings[off].compare_exchange_weak(x, data, std::memory_order_release, std::memory_order_relaxed)) {
          delete data;
          return cmpxchg(pos, oldVal, newVal);
        }
        return true;
      }
    } else {
      std::atomic<value_type&> atom(_append[pos - _allocSize]);
      return atom.compare_exchange_weak(oldVal, newVal, std::memory_order_release, std::memory_order_relaxed);
    }
  }

  // Resizes are always materialized
  void resize(size_t count) {
    resize(count, _defaultValue);
  }

  void push_back(const T& v) {
    _append.push_back(v);
  }

  template< class... Args >
  void emplace_back( Args&&... args ) {
    _append.emplace_back(std::forward<Args...>(args...));
  }  

  iterator begin() {
    return iterator(this, 0);
  }

  iterator end() {
    return iterator(this, size());
  }

  const_iterator begin() const {
    return const_iterator(this, 0);
  }

  const_iterator end() const {
    return const_iterator(this, size());
  }

  const_iterator cbegin() const {
    return const_iterator(this, 0);
  }

  const_iterator cend() const {
    return const_iterator(this, size());
  }  


};

/**
 */
template<typename V>
class SparseVectorIterator : std::iterator<std::forward_iterator_tag, typename V::value_type> {
public:

  using iterator = SparseVectorIterator<V>;
  using vector_type = V;
  using value_type = typename V::value_type;

  V * _vector;
  size_t _index;

  explicit SparseVectorIterator(V* v, size_t index) : _vector(v), _index(index) {
  }

  bool operator== (const iterator& other) const {
    return _vector == other._vector && _index == other._index;
  }

  bool operator!= (const iterator& other) const {
    return !(operator==(other));
  }

  value_type operator* () const {
    return _vector->get(_index);
  }

  value_type operator-> () const {
    return _vector->get(_index);
  }

  iterator& operator++() {
    ++_index;
    return *this;
  }

  iterator operator++(int) {
    auto res = *this;
    ++_index;
    return res;
  }

};


}}
