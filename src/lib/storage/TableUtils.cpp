// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/TableUtils.h"

#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

column_mapping_t calculateMapping(const std::shared_ptr<const AbstractTable> &source,
                                  const std::shared_ptr<const AbstractTable> &dest) {
  return calculateMapping(*source.get(), *dest.get());
}

column_mapping_t calculateMapping(
    const AbstractTable& input,
    const AbstractTable& dest) {
  column_mapping_t map;
  for (size_t column_index = 0; column_index < input.columnCount(); ++column_index) {
    map[column_index] = dest.numberOfColumn(input.nameOfColumn(column_index));
  }
  return map;
}

}}

