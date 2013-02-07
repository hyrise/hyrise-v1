// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_LOADEREXCEPTION_H_
#define SRC_LIB_IO_LOADEREXCEPTION_H_

#include <stdexcept>
#include <string>

namespace Loader {
class Error : public std::runtime_error {
 public:
  explicit Error(std::string msg) : std::runtime_error(msg)
  {}
};
}

#endif  // SRC_LIB_IO_LOADEREXCEPTION_H_
