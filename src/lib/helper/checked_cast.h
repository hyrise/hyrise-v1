// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_HELPER_CHECKED_CAST_H_
#define SRC_LIB_HELPER_CHECKED_CAST_H_

#include <memory>

template <class T, class U>
inline std::shared_ptr<T> checked_pointer_cast (const std::shared_ptr<U>& sp) {
  assert((std::dynamic_pointer_cast<T>(sp) != nullptr) && "Cast failed");
  return std::dynamic_pointer_cast<T>(sp);
}

#endif
