// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include <storage/BaseIterator.h>

#include <boost/iterator/iterator_facade.hpp>

namespace hyrise {
namespace storage {

template<typename T>
class DictionaryIterator : public boost::iterator_facade<DictionaryIterator<T>, T, boost::forward_traversal_tag> {

 public:

  typedef BaseIterator<T> base_it_t;
  typedef std::shared_ptr<base_it_t> shared_it_ptr_t;
  shared_it_ptr_t _it;

  DictionaryIterator(): _it(nullptr) {}

  explicit DictionaryIterator(shared_it_ptr_t it): _it(it) {
    //std::cout << "dict it constr" << std::endl;
  }

  virtual ~DictionaryIterator() {}

  void increment() {
    _it->increment();
  }

  T &dereference() const {
    return _it->dereference();
  }

  bool equal(const DictionaryIterator& other) const {
    return _it->equal(other._it);
  }

  value_id_t getValueId() const {
    return _it->getValueId();
  }

};

} } // namespace hyrise::storage

