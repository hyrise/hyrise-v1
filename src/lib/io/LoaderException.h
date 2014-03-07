// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdexcept>
#include <string>

namespace hyrise {
namespace io {
namespace Loader {

class Error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

} } } // namespace hyrise::io::Loader

