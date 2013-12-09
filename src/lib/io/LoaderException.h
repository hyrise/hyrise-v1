// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdexcept>
#include <string>

namespace hyrise {
namespace io {
namespace Loader {

class Error : public std::runtime_error {
 public:
  explicit Error(std::string msg) : std::runtime_error(msg)
  {}
};

} } } // namespace hyrise::io::Loader

