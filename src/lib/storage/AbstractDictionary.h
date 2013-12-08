// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <string>
#include <memory>

#include "storage/storage_types.h"

namespace hyrise {
namespace storage {

class AbstractDictionary {
public:
  virtual ~AbstractDictionary() {}

  virtual bool isOrdered() = 0;

  virtual void reserve(size_t size) = 0;

  virtual std::shared_ptr<AbstractDictionary> copy() = 0;
  virtual std::shared_ptr<AbstractDictionary> copy_empty() = 0;
  virtual size_t size() = 0;

  virtual void shrink() = 0;

};

} } // namespace hyrise::storage

