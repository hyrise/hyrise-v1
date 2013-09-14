// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_BASEITERATOR_H_
#define SRC_LIB_STORAGE_BASEITERATOR_H_

#include <memory>

template<typename T>
class BaseIterator {

 public:

  virtual ~BaseIterator() {}

  virtual void increment() = 0;

  virtual T &dereference() const = 0;

  virtual bool equal(const std::shared_ptr<BaseIterator<T>>& other) const = 0;

  virtual value_id_t getValueId() const = 0;

};

#endif  // SRC_LIB_STORAGE_BASEITERATOR_H_
