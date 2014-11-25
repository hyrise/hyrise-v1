// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <boost/lexical_cast.hpp>

#include "storage/storage_types.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

template <typename T>
struct cast_functor_by_row {
  typedef T value_type;

  AbstractTable* table;
  size_t column;
  size_t row;

  cast_functor_by_row(AbstractTable* t, const size_t f, const size_t r) : table(t), column(f), row(r) {}

  template <typename R>
  T operator()() {
    return boost::lexical_cast<T>(table->getValue<R>(column, row));
  }
};
}
}
