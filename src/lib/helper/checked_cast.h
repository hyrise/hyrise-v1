// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include "demangle.h"

template <class T, class U>
inline std::shared_ptr<T> checked_pointer_cast(const std::shared_ptr<U>& sp) {
  auto result = std::dynamic_pointer_cast<T>(sp);
  if (!result) {
    std::string s1(typeid(*sp).name());
    std::string s2(typeid(T).name());
    std::stringstream ss;

    ss << "checked_pointer_cast failed: Pointer is not of requested type: Tried to convert " << demangle(s1.c_str())
       << " (" << s1 << ") to " << demangle(s2.c_str()) << " (" << s2 << ")";
    throw std::runtime_error(ss.str());
  }
  return result;
}
