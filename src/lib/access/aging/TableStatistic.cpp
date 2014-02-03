// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableStatistic.h"

#include <iostream>

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <access/aging/QueryManager.h>

namespace hyrise {
namespace access {

TableStatistic::TableStatistic(storage::atable_ptr_t table, storage::field_t field,
                               storage::c_atable_ptr_t statisticTable) :
    storage::AbstractStatistic(table, field),
    _statisticTable(statisticTable) {
  if (table->typeOfColumn(field) != statisticTable->typeOfColumn(statisticTable->numberOfColumn("value")))
    throw std::runtime_error("Type of tables field and statistic tables value field does not match");
}

TableStatistic::~TableStatistic() {}

namespace {

template <typename T>
bool isHotHelper(storage::atable_ptr_t table, storage::c_atable_ptr_t statisticTable,
                 storage::field_t field, const std::string& query, storage::value_id_t vid) {
  const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));
  const T value = dict->getValueForValueId(vid);

  const auto valueField = statisticTable->numberOfColumn("value");
  const auto& statisticDict = checked_pointer_cast<storage::BaseDictionary<T>>(statisticTable->dictionaryAt(valueField));
  const auto& statisticVid = statisticDict->getValueIdForValue(value);

  const auto size = statisticTable->size();

  for (size_t row = 0; row < size; ++row) {
    if (statisticTable->getValueId(valueField, row).valueId == statisticVid)
      return statisticTable->getValue<hyrise_int_t>(query, row);
  }

  return false;
  //TODO we might need some exception handling
}

} // namespace

bool TableStatistic::isHot(const std::string& query, value_id_t vid) const {
  if (!isRegistered(query)) {
    std::cout << ">>>>unregistered value! (\"" << query << "\", field: " << table()->nameOfColumn(field())
              << ", vid: " << vid << ")" << std::endl;
    return false;
  }

  switch (table()->typeOfColumn(field())) {
    case IntegerType:
      return isHotHelper<hyrise_int_t>(table(), _statisticTable, field(), query, vid);
      break;
    case FloatType:
      return isHotHelper<hyrise_float_t>(table(), _statisticTable, field(), query, vid);
      break;
    case StringType:
      return isHotHelper<hyrise_string_t>(table(), _statisticTable, field(), query, vid);
      break;
    default:
      throw std::runtime_error("unsupported type");
  }
}

bool TableStatistic::isHot(query_t query, value_id_t value) const {
  const auto& qm = QueryManager::instance();
  return isHot(qm.getName(query), value);
}

bool TableStatistic::isRegistered(const std::string& query) const {
  try {
    _statisticTable->numberOfColumn(query);
    return true;
  }
  catch(...) {}
  return false;
}

bool TableStatistic::isRegistered(query_t query) const {
  const auto& qm = QueryManager::instance();
  return isRegistered(qm.getName(query));
}

namespace {

template <typename T>
void valuesDoHelper(storage::atable_ptr_t table, storage::c_atable_ptr_t statisticTable,
                    storage::field_t field, std::function<void(query_t, storage::value_id_t, bool)> func) {
  const auto& valueField = statisticTable->numberOfColumn("value");
  //TODO right dict function?
  const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));
  const auto& qm = QueryManager::instance();

  const size_t size = statisticTable->size();
  const auto fieldc = statisticTable->columnCount();

  for (size_t col = 0; col < fieldc; ++col) {
    const auto& queryName = statisticTable->nameOfColumn(col);
    if (col == valueField || !qm.exists(queryName)) continue;
    const query_t query = qm.getId(queryName);

    for (size_t row = 0; row < size; ++row) {
      //I suspect inverse dict access to be terribly slow
      const T value = statisticTable->getValue<T>(valueField, row);
      const auto valueId = dict->getValueIdForValue(value);
      const bool hotVal = statisticTable->getValue<hyrise_int_t>(col, row); //0 cold, 1 hot
      func(query, valueId, hotVal);
    }
  }
}

} // namespace

void TableStatistic::valuesDo(std::function<void(query_t, storage::value_id_t, bool)> func) const {
  switch (table()->typeOfColumn(field())) {
    case IntegerType: valuesDoHelper<hyrise_int_t>   (table(), _statisticTable, field(), func); break;
    case FloatType:   valuesDoHelper<hyrise_float_t> (table(), _statisticTable, field(), func); break;
    case StringType:  valuesDoHelper<hyrise_string_t>(table(), _statisticTable, field(), func); break;
    default: throw std::runtime_error("unsupported field type");
  }
}

} } // namespace hyrise::access

