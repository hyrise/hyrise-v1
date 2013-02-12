// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_HASH_FUNCTOR_H_
#define SRC_LIB_STORAGE_HASH_FUNCTOR_H_

#include <functional>

#include "helper/types.h"
#include "storage/storage_types.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

template<typename T>
struct hash_functor {
  typedef T value_type;
  const hyrise::storage::DCMutableTable *table;
  size_t f;
  ValueId vid;

  hash_functor(): table(0) {}

  hash_functor(const hyrise::storage::DCMutableTable *t, const size_t f, const ValueId v): table(t), f(f), vid(v) {}

  template<typename R>
  T operator()() {
    return std::hash<R>()(table->getValueForValueId<R>(f, vid));
  }
};

}}

#endif
