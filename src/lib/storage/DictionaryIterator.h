// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_DICTIONARYITERATOR_H_
#define SRC_LIB_STORAGE_DICTIONARYITERATOR_H_

#include <storage/BaseIterator.h>

#include <boost/iterator/iterator_facade.hpp>

template<typename T>
class DictionaryIterator : public boost::iterator_facade<DictionaryIterator<T>, T, boost::forward_traversal_tag> {

 public:

  BaseIterator<T> *_it;

  DictionaryIterator(): _it(nullptr) {}

  explicit DictionaryIterator(BaseIterator<T> *it): _it(it) {
    //std::cout << "dict it constr" << std::endl;
  }

  DictionaryIterator(const DictionaryIterator &other) {
    //std::cout << "copy constr dict it" << std::endl;
    _it = other._it->clone();
  }

  DictionaryIterator &operator = (const DictionaryIterator &other) {
    if (this != &other) {
      delete _it;
      //std::cout << "assign dict it" << std::endl;
      _it = other._it->clone();
    }

    return *this;
  }

  virtual ~DictionaryIterator() {
    //std::cout << "~DI" << std::endl;
    delete _it;
  }

  void increment() {
    _it->increment();
  }

  T &dereference() const {
    return _it->dereference();
  }

  bool equal(const DictionaryIterator<T> &other) const {
    return _it->equal(other._it);
  }

  value_id_t getValueId() const {
    return _it->getValueId();
  }

};

#endif  // SRC_LIB_STORAGE_DICTIONARYITERATOR_H_
