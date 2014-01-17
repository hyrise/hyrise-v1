// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableStatistic.h"

#include <iostream>

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <access/aging/QueryManager.h>

namespace hyrise {
namespace access {

TableStatistic::TableStatistic(storage::atable_ptr_t table) :
    storage::AbstractStatistic(table) {}

TableStatistic::~TableStatistic() {}

void TableStatistic::addStatisticTable(const std::string& field, storage::c_atable_ptr_t statisticTable, bool overRide) {
  const auto& fieldId = table()->numberOfColumn(field);
  addStatisticTable(fieldId, statisticTable, overRide);
}

void TableStatistic::addStatisticTable(field_t field, storage::c_atable_ptr_t statisticTable, bool overRide) {
  if (!overRide && _tables.find(field) != _tables.end()) {
    std::cout << "statistic table already exists - merging not implemented" << std::endl;
    return;
  }

  std::cout << ">>>>>>>>>>>>>save statistics" << std::endl;
  const auto valueField = statisticTable->numberOfColumn("value"); //just checking ...
  if (statisticTable->typeOfColumn(valueField) != table()->typeOfColumn(field))
    throw std::runtime_error("invalid statistic table: \"value\" column needs to have same type as table column");

  const size_t fieldc = statisticTable->columnCount();
  for (size_t col = 0; col < fieldc; ++col) {
    if (col != valueField && statisticTable->typeOfColumn(col) != IntegerType)
      throw std::runtime_error("hotness values may only be stored in INTEGER columns");
  }

  _tables[field] = statisticTable;
}

bool TableStatistic::isHot(const std::string& query, field_t field, value_id_t value) const {
  if (!isRegistered(query, field)) {
    std::cout << ">>>>unregistered value!" << std::endl;
    return false;
  }
  const auto statisticTable = _tables.at(field);
  //return statisticTable.getValue<hyrise_int_t>(query,
  //TODO
  return false;
}

bool TableStatistic::isHot(query_t query, field_t field, value_id_t value) const {
  const auto& qm = QueryManager::instance();
  return isHot(qm.getName(query), field, value);
}

bool TableStatistic::isRegistered(const std::string& query, field_t field) const {
  const auto& table = _tables.find(field);
  if (table == _tables.end())
    return false;
  try {
    table->second->numberOfColumn(query);
    return true;
  }
  catch(...) {}
  return false;
}

bool TableStatistic::isRegistered(query_t query, field_t field) const {
  const auto& qm = QueryManager::instance();
  return isRegistered(qm.getName(query), field);
}

namespace {

template <typename T>
void valuesDoHelper(storage::atable_ptr_t table, storage::field_t field, storage::c_atable_ptr_t statisticTable,
                    std::function<void(query_t, storage::field_t, storage::value_id_t, bool)> func) {
  const auto& valueField = statisticTable->numberOfColumn("value");
  std::cout << statisticTable->typeOfColumn(valueField) << ";" << table->typeOfColumn(field) << std::endl;
  //TODO right dict function?
  const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));
  const auto& qm = QueryManager::instance();

  const size_t size = statisticTable->size();
  const auto fieldc = statisticTable->columnCount();
  std::cout << "==========" << size << "====" << fieldc << std::endl;

  for (size_t col = 0; col < fieldc; ++col) {
    const auto& queryName = statisticTable->nameOfColumn(col);
    if (col == valueField || !qm.exists(queryName)) continue;
    std::cout << "DDDDDDDDDDDDDDDDDDDDDDDD" << std::endl;
    const query_t query = qm.getId(queryName);

    for (size_t row = 0; row < size; ++row) {
      //I suspect inverse dict access to be terribly slow
      const T value = statisticTable->getValue<T>(valueField, row);
      std::cout << value << "<<<<<<<" << std::endl;
      const auto valueId = dict->getValueIdForValue(value);
      const bool hotVal = statisticTable->getValue<hyrise_int_t>(col, row); //0 cold, 1 hot
      func(query, field, valueId, hotVal);
    }
  }
}

} // namespace

void TableStatistic::valuesDo(std::function<void(query_t, storage::field_t, storage::value_id_t, bool)> func) const {
  for (const auto& t : _tables) {
    const auto& field = t.first;
    const auto& statisticTable = t.second;
    switch (table()->typeOfColumn(field)) {
      case IntegerType: valuesDoHelper<hyrise_int_t>   (table(), field, statisticTable, func); break;
      case FloatType:   valuesDoHelper<hyrise_float_t> (table(), field, statisticTable, func); break;
      case StringType:  valuesDoHelper<hyrise_string_t>(table(), field, statisticTable, func); break;
      default: throw std::runtime_error("unsupported field type");
    }
  }
}

} } // namespace hyrise::access

