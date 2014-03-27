// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <stdexcept>
#include <iostream>

template <class T, class U>
inline std::shared_ptr<T> checked_pointer_cast(const std::shared_ptr<U>& sp) {
  auto result = std::dynamic_pointer_cast<T>(sp);
  if (!result) {
#ifndef NDEBUG
    std::string s1(typeid(*sp).name());
    std::string s2(typeid(T).name());
    throw std::runtime_error("Pointer is not of requested type got a \" " + s1 + " \" cannot make it a \" " + s2 +
                             "\" ");
#else
    throw std::runtime_error("Pointer is not of requested type");
#endif
  }
  return result;
}
