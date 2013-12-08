// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/EmptyLoader.h"

namespace hyrise {
namespace io {

std::shared_ptr<storage::AbstractTable> EmptyInput::load(std::shared_ptr<storage::AbstractTable> table,
                                                         const storage::compound_metadata_list *t,
                                                         const Loader::params &args) {
  return table;
}

EmptyInput *EmptyInput::clone() const {
  return new EmptyInput(*this);
}

storage::compound_metadata_list *EmptyHeader::load(const Loader::params &args) {
  auto *l = new storage::compound_metadata_list();
  return l;
}

EmptyHeader *EmptyHeader::clone() const {
  return new EmptyHeader(*this);
}

} } // namespace hyrise::io

