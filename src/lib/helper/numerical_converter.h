#pragma once

#include "optional.hpp"

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
std::optional<toType> parseNumeric(const std::string& s) {
  std::optional<toType> value;
  char * endptr;
  value = numerical_detail::converter<toType>::func(s.c_str(), &endptr, 10);
  if (&(*s.end()) != endptr) { value = std::nullopt; }
  return value;
}

