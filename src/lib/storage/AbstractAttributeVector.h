// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstddef>

namespace hyrise {
namespace storage {

class AbstractAttributeVector {
 public:

  virtual ~AbstractAttributeVector();

  virtual void *data() = 0;
  virtual void setNumRows(size_t s) = 0;

};

} } // namespace hyrise::storage

