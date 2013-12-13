// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/VectorLoader.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace io {

VectorInput::VectorInput(value_vectors_t vectors) : _vectors(vectors) {}

VectorInput* VectorInput::clone() const {
  return new VectorInput(*this);
}

std::shared_ptr<storage::AbstractTable> VectorInput::load(std::shared_ptr<storage::AbstractTable> table,
                                                          const storage::compound_metadata_list * meta,
                                                          const Loader::params &args) {
  size_t column = 0;
  for (const auto& column_values: _vectors) {
    size_t row = 0;
    table->resize(column_values.size());
    for (const auto& value: column_values) {
      table->setValue<hyrise_int_t>(column, row, value);
      ++row;
    }
    ++column;
  }
  return table;
}

} } // namespace hyrise::io

