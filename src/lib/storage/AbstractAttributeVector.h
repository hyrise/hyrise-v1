// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstddef>

namespace hyrise {
namespace storage {

class AbstractAttributeVector {
 public:
  virtual ~AbstractAttributeVector();
  virtual size_t getColumns() const = 0;
};
}
}  // namespace hyrise::storage
