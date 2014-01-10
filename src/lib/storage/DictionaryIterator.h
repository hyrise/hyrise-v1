// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <iterator>


#include <storage/BaseIterator.h>



namespace hyrise {
namespace storage {

template<typename T>
class DictionaryIterator : public std::iterator<std::forward_iterator_tag, T> {

 public:

  typedef BaseIterator<T> base_it_t;
  typedef std::shared_ptr<base_it_t> shared_it_ptr_t;
  shared_it_ptr_t _it;

  // Public Typedef
  using iterator = DictionaryIterator<T>;

  DictionaryIterator(): _it(nullptr) {}

  explicit DictionaryIterator(shared_it_ptr_t it): _it(it) {
  }

  virtual ~DictionaryIterator() {}

  bool operator==(const DictionaryIterator& other) const  {
    return _it->equal(other._it);
  }

  bool operator!=(const DictionaryIterator& other) const {
    return !(operator==(other));
  }

  T operator*() {
    return _it->dereference();
  }

  iterator& operator++() {
    _it->increment();
    return *this;
  }

  iterator operator++(int) {
    auto res = *this;
    _it->increment();
    return res;
  }
  

  value_id_t getValueId() const {
    return _it->getValueId();
  }

};

} } // namespace hyrise::storage

