// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_META_STORAGE_H_
#define SRC_LIB_STORAGE_META_STORAGE_H_

#include <stdexcept>

#include "boost/mpl/at.hpp"
#include "boost/mpl/int.hpp"
#include "boost/mpl/size.hpp"

#include "storage/storage_types.h"

namespace hyrise {
namespace storage {

typedef  boost::mpl::size<hyrise_basic_types> basic_types_size;

/*
  This is a simple implementation of a list based type switch. Based on the main
  defintion of all available types this template defintion recurses through to
  find the correct type and based on this type call the functor
*/
template <typename L, int N = 0, bool Stop = (N == basic_types_size::value)> struct type_switch;

template <typename L, int N, bool Stop>
struct type_switch {
  template<class F>
  inline typename F::value_type operator()(size_t i, F &f) {
    if (i == N) {
      return f.operator()<typename boost::mpl::at_c< hyrise_basic_types, N>::type>();
    } else {
      type_switch < L, N + 1 > next;
      return next(i, f);
    }
  }
};

template <typename L, int N>
struct type_switch<L, N, true> {
  template<class F>
  inline typename F::value_type operator()(size_t i, F &f) {
    throw std::runtime_error("Type does not exist");
  }
};


}}

#endif  // SRC_LIB_STORAGE_META_STORAGE_H_

