// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "io/AbstractLoader.h"

namespace hyrise {
namespace io {

class EmptyInput : public AbstractInput {
 public:
  EmptyInput() {
  }
  EmptyInput *clone() const;
  std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable>, const storage::compound_metadata_list *, const Loader::params &args);
};

class EmptyHeader : public AbstractHeader {
 public:
  EmptyHeader() {
  }
  EmptyHeader *clone() const;
  storage::compound_metadata_list *load(const Loader::params &args);
};

} } // namespace hyrise::io

