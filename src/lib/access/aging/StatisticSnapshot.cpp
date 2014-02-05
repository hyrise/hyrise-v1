// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "StatisticSnapshot.h"

#include <access/aging/QueryManager.h>

namespace hyrise {
namespace access {

namespace {

value_id_map_t createVidMap(const std::vector<storage::value_id_t>& vids) {
  value_id_map_t map;
  size_t cur = 0;

  for (const auto& vid : vids) {
    const auto x = map.insert(std::make_pair(vid, cur++));
    if (!x.second)
      throw std::runtime_error("value id exists multiple times!");
  }

  return map;
}

} // namespace

StatisticSnapshot::StatisticSnapshot(const storage::AbstractStatistic& other) :
    AbstractStatistic(other),
    _vidMap(createVidMap(other.vids())) {
  //TODO create hotnessMap
}

bool StatisticSnapshot::isHot(query_t query, storage::value_id_t value) const {
  const auto& hotnessVector = _hotnessMap.find(query);
  if (hotnessVector == _hotnessMap.cend())
    return false;

  const auto& internalId = _vidMap.find(value);
  if (internalId == _vidMap.cend())
    return false;
  
  return hotnessVector->second.at(internalId->second);
}

bool StatisticSnapshot::isQueryRegistered(query_t query) const {
  return _hotnessMap.find(query) != _hotnessMap.cend();
}

bool StatisticSnapshot::isValueRegistered(storage::value_id_t vid) const {
  return _vidMap.find(vid) != _vidMap.cend();
}

void StatisticSnapshot::valuesDo(query_t query, std::function<void(storage::value_id_t, bool)> func) const {
  if (!isQueryRegistered(query)) {
    //TODO maybe delete for performance
    const auto& qm = QueryManager::instance();
    if (!qm.exists(qm.getName(query)))
      throw std::runtime_error("QueryID does not exists");
    return;
  }
  const auto& hotnessVector = _hotnessMap.at(query);
  for (const auto& vid : _vidMap)
    func(vid.first, hotnessVector.at(vid.second));
}

std::vector<query_t> StatisticSnapshot::queries() const {
  std::vector<query_t> ret(_hotnessMap.size());
  size_t cur = 0;
  for (const auto& query : _hotnessMap)
    ret[cur++] = query.first;
  return ret;
}

std::vector<storage::value_id_t> StatisticSnapshot::vids() const {
  std::vector<storage::value_id_t> ret(_vidMap.size());
  size_t cur = 0;
  for (const auto& vid : _vidMap)
    ret[cur++] = vid.first;
  return ret;
}

} } // namespace hyrise::access

/*
#include <iostream>

#include <storage/storage_types_helper.h>
#include <storage/BaseDictionary.h>

namespace hyrise {
namespace storage {

namespace {
  template <typename T>
  static AgingIndex::value_id_map_t createVIdMap(const atable_ptr_t& table, field_t column) {
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

AgingIndex::AgingIndex(const atable_ptr_t& table) :
    _table(table) {
  updateValueIdMapping();
}

AgingIndex::~AgingIndex() {}

void AgingIndex::shrink() {
  //TODO ... implement it?
}

void AgingIndex::addForQuery(access::query_t query, const std::vector<field_t>& fields) {
  const auto& pvVectors = table()->getAttributeVectors(table()->numberOfColumn("$PV"));
  value_id_t hotValue = 1; //TODO not best

  std::cout << ">>>>>>>>>>>>>>>updating AgingIndex" << std::endl;
  for (const auto& field : fields) {
    if (_hotnessTable.find(field) == _hotnessTable.end())
      throw std::runtime_error("Field is not registered for aging");
    auto& hotnessMap = _hotnessTable.find(field)->second;

    if (hotnessMap.find(query) != hotnessMap.end())
      throw std::runtime_error("Query is already registered");
    const auto& valueMap = _valueIdTable.find(field)->second;

    const auto& fieldVectors = table()->getAttributeVectors(field);
    AgingIndex::hotness_vector_t hotnessVector(valueMap.size(), true);

    // maybe later more tables
    const auto& fieldVector = checked_pointer_cast<storage::BaseAttributeVector<value_id_t>>(fieldVectors.at(0).attribute_vector);
    const auto& pvVector = checked_pointer_cast<storage::BaseAttributeVector<value_id_t>>(pvVectors.at(0).attribute_vector);

    for (unsigned i = 0; i < fieldVector->size(); ++i) {
      const auto& valueId = fieldVector->get(0, i);
      const auto& internalId = valueMap.find(valueId)->second;
      bool hotval = hotnessVector.at(internalId);
      hotnessVector.at(internalId) = hotval & (pvVector->get(0, i) == hotValue);
    }

    for (const auto& hot : hotnessVector)
      std::cout << ">>" << (hot ? "HOT" : "COLD") << std::endl;

    std::cout << "create hotness Vector for Query " << query << " and field " << field << std::endl;

    hotnessMap.insert(std::make_pair(query, hotnessVector));
  }
}

bool AgingIndex::isHot(access::query_t query, std::vector<param_t> params) {
  bool hot = true;
  for (const auto& param : params) {
    if (_hotnessTable.find(param.field) == _hotnessTable.end())
      throw std::runtime_error("field id not saved in AgingIndex");
    const auto& hotnessMap = _hotnessTable.find(param.field)->second;

    if (hotnessMap.find(query) == hotnessMap.end())
      throw std::runtime_error("query not registered for this field and table");
    const auto& hotnessVector = hotnessMap.find(query)->second;

    const auto& internalId = _valueIdTable.find(param.field)->second.find(param.vid)->second;

    hot &= hotnessVector.at(internalId);
  }
  return hot;
}

atable_ptr_t AgingIndex::table() {
  return _table.lock();
}

AgingIndex::hotness_map_t& AgingIndex::findOrCreate(field_t field) {
  const auto& it = _hotnessTable.find(field);
  if (it == _hotnessTable.end()) {
    AgingIndex::hotness_map_t map;
    _hotnessTable.insert(std::make_pair(field, map));
    return _hotnessTable.at(field);
  }
  return it->second;
}

AgingIndex::hotness_vector_t& AgingIndex::findOrCreate(field_t field, access::query_t query) {
  auto& hotnessMap = findOrCreate(field);
  
  const auto& it = hotnessMap.find(field);
  if (it == hotnessMap.end()) {
    AgingIndex::hotness_vector_t vector;
    hotnessMap.insert(std::make_pair(field, vector));
    return hotnessMap.at(query);
  }
  return it->second;
}

void AgingIndex::add(access::query_t query, field_t field, value_id_t value, bool hot) {
  const auto& valueMap = _valueIdTable.find(field);
  if (valueMap == _valueIdTable.end())
    throw std::runtime_error("valueMap does not exist");
  const auto& internalId = valueMap->second.find(value);
  if (internalId == valueMap->second.end())
    throw std::runtime_error("value is not mapped in valueMap ... missed an update()?");

  auto& hotnessVector = findOrCreate(field, query);
  hotnessVector.at(internalId->second) = hot;
}

void AgingIndex::updateValueIdMapping() {
  //TODO make it usable for multiple calls
  const field_t numberOfFields = table()->columnCount();
  for (field_t i = 0; i < numberOfFields; ++i) {
    if (table()->nameOfColumn(i).at(0) == '$') continue;

    const auto type = table()->typeOfColumn(i);
    std::cout << "take column: " << table()->nameOfColumn(i) << " (" << data_type_to_string(type) << ")" << std::endl;

    value_id_map_t vIdMap;
    switch (type) {
      case IntegerType: vIdMap = createVIdMap<hyrise_int_t>(table(), i); break;
      case FloatType:   vIdMap = createVIdMap<hyrise_float_t>(table(), i); break;
      case StringType:  vIdMap = createVIdMap<hyrise_string_t>(table(), i); break;
    }
    _valueIdTable.insert(std::make_pair(i, vIdMap));

    hotness_map_t hotnessMap;
    _hotnessTable.insert(std::make_pair(i, hotnessMap));
  }
}

} } // namespace hyrise::storage
*/
