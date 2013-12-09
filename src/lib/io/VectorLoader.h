// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "io/AbstractLoader.h"

namespace hyrise {
namespace io {

class VectorInput : public AbstractInput {
 public:
  typedef std::vector<std::uint64_t> value_vec_t;
  typedef std::vector<value_vec_t> value_vectors_t;
  VectorInput(value_vectors_t vectors);
  VectorInput *clone() const;
  std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable>,
                                               const storage::compound_metadata_list *,
                                               const Loader::params &args);
 private:
  const value_vectors_t _vectors;
};

} } // namespace hyrise::io

