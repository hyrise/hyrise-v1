// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AgingIndex.h>

#include <iostream>

#include <storage/storage_types_helper.h>
#include <storage/BaseDictionary.h>

namespace hyrise {
namespace storage {

AgingIndex::AgingIndex(const atable_ptr_t& table) {
  const field_t numberOfFields = table->columnCount();
  for (field_t i = 0; i < numberOfFields; ++i) {
    if (table->nameOfColumn(i).at(0) == '$') continue;

    const auto type = table->typeOfColumn(i);
    std::cout << "take column: " << table->nameOfColumn(i) << " (" << data_type_to_string(type) << ")" << std::endl;

    value_id_map_t vIdMap;
    switch (type) {
      case IntegerType: vIdMap = createVIdMap<hyrise_int_t>(table, i); break;
      case FloatType:   vIdMap = createVIdMap<hyrise_float_t>(table, i); break;
      case StringType:  vIdMap = createVIdMap<hyrise_string_t>(table, i); break;
    }
    _value_id_table.insert(std::make_pair(i, vIdMap));

    hotness_map_t hotness;

    _hotness_table.insert(std::make_pair(i, hotness));
  }
}

AgingIndex::~AgingIndex() {}

void AgingIndex::shrink() {}

bool isHot(access::query_id_t query, std::vector<value_id_t> values) {
  return false;
}

} } // namespace hyrise::storage

