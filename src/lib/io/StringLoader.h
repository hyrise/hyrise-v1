// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "io/AbstractLoader.h"

namespace hyrise {
namespace io {

class StringHeader : public AbstractHeader {
 public:
  explicit StringHeader(std::string header) :
      _header(header) {
  }
  storage::compound_metadata_list *load(const Loader::params &args);
  StringHeader *clone() const;
 private:
  std::string _header;
};

} } // namespace hyrise::io

