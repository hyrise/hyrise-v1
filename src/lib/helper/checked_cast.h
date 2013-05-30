// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_HELPER_CHECKED_CAST_H_
#define SRC_LIB_HELPER_CHECKED_CAST_H_

#include <memory>
#include <stdexcept>

template <class T, class U>
inline std::shared_ptr<T> checked_pointer_cast (const std::shared_ptr<U>& sp) {
  auto result = std::dynamic_pointer_cast<T>(sp);
  if (!result) throw std::runtime_error("Pointer is not of requested type");
  return result;
}

#endif
