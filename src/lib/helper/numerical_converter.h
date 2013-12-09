#pragma once

#include "boost/optional.hpp"

namespace numerical_detail {

template <typename T>
struct converter;

#define CONVERTER(TYPE, FUNCTION)                               \
  template <>                                                   \
  struct converter<TYPE> {                                      \
    static constexpr decltype(&FUNCTION) func = &FUNCTION;      \
  }


CONVERTER(unsigned long, std::strtoul);
CONVERTER(unsigned long long, std::strtoull);
CONVERTER(int, std::strtol);
CONVERTER(long, std::strtol);
CONVERTER(long long, std::strtoll);

#undef CONVERTER

}

template <typename toType>
boost::optional<toType> parseNumeric(const std::string& s) {
  boost::optional<toType> value;
  char * endptr;
  value = numerical_detail::converter<toType>::func(s.c_str(), &endptr, 10);
  if (&(*s.end()) != endptr) { value = boost::optional<toType>(); }
  return value;
}

