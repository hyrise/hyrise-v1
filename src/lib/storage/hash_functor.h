// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <functional>

#include "storage/storage_types.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

template<typename T>
struct hash_functor {
  typedef T value_type;
  const AbstractTable *table;
  size_t f;
  ValueId vid;

  hash_functor(): table(0) {}

  void setValueId(ValueId v) {
    vid = vid;
  }

  hash_functor(const AbstractTable *t, const size_t f, const ValueId v=ValueId()): table(t), f(f), vid(v) {}

  template<typename R>
  T operator()() {
    return std::hash<R>()(table->getValueForValueId<R>(f, vid));
  }
};

}}

