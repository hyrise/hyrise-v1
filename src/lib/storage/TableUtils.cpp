// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/TableUtils.h"

#include "helper/types.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

column_mapping_t calculateMapping(const hyrise::storage::c_atable_ptr_t &source,
                                  const hyrise::storage::c_atable_ptr_t &dest) {
  column_mapping_t map; 	
  for (size_t column_index = 0; column_index < source->columnCount(); ++column_index) {
    map[column_index] = dest->numberOfColumn(source->nameOfColumn(column_index));
  }
  return map;
}

}}
