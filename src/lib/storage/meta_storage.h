// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdexcept>

#include "boost/mpl/at.hpp"
#include "boost/mpl/int.hpp"
#include "boost/mpl/size.hpp"

#include "storage/storage_types.h"

#include "boost/mpl/vector.hpp"
//#include "boost/mpl/map.hpp"

// This are the basic types used in HYRISE, the enum above can
// be used to directly offset into the list at compile time
//
// ATTENTION: Any new types in the enum have to reflected here
typedef boost::mpl::vector<hyrise_int_t, hyrise_float_t, hyrise_string_t,
			   // Delta Types
			   hyrise_int_t, hyrise_float_t, hyrise_string_t,
			   // Concurrent Delta Types,
			   hyrise_int_t, hyrise_float_t, hyrise_string_t,
			   // No Dict Types
			   hyrise_int32_t, hyrise_float_t > hyrise_basic_types;

namespace hyrise {
namespace storage {

typedef  boost::mpl::size<hyrise_basic_types> basic_types_size;

/*
  This is a simple implementation of a list based type switch. Based on the main
  defintion of all available types this template defintion recurses through to
  find the correct type and based on this type call the functor
*/
template <typename L, int N = 0, bool Stop = (N == boost::mpl::size<L>::value)> struct type_switch;

template <typename L, int N, bool Stop>
struct type_switch {
  template<class F>
  inline typename F::value_type operator()(size_t i, F &f) {
    if (i == N) {
      return f.template operator()<typename boost::mpl::at_c< L, N>::type>();
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

