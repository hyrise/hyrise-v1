// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AgingIndex.h>

#include <iostream>

#include <storage/storage_types_helper.h>
#include <storage/BaseDictionary.h>

namespace hyrise {
namespace storage {

namespace {
  template <typename T>
  static AgingIndex::value_id_map_t createVIdMap(const atable_ptr_t& table, field_t column){
    AgingIndex::value_id_map_t ret;
    //TODO maybe better function for getting the dictionary
    const auto& dict = checked_pointer_cast<BaseDictionary<T>>(table->dictionaryAt(column));

    auto it = dict->begin();
    const auto& end = dict->end();
    unsigned count = 0;
    while (it != end) {
       ret.insert(std::make_pair(it.getValueId(), count++));
       ++it;
    }

    return ret;
  }
} // namespace

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
    _valueIdTable.insert(std::make_pair(i, vIdMap));

    hotness_map_t hotnessMap;

    //TODO create hotnessMap

    _hotnessTable.insert(std::make_pair(i, hotnessMap));
  }
}

AgingIndex::~AgingIndex() {}

void AgingIndex::shrink() {}

bool AgingIndex::isHot(access::query_id_t query, field_t field, value_id_t vid) {
  if (_hotnessTable.find(field) == _hotnessTable.end())
    throw std::runtime_error("field id not saved in AgingIndex");
  const auto& hotnessMap = _hotnessTable.find(field)->second;

  if (hotnessMap.find(query) == hotnessMap.end())
    throw std::runtime_error("query not registered for this field and table");
  const auto& hotnessVector = hotnessMap.find(query)->second;

  const auto& internalId = _valueIdTable.find(field)->second.find(vid)->second;

  return hotnessVector.at(internalId);
}

} } // namespace hyrise::storage

