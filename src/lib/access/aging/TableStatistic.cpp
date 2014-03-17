// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableStatistic.h"

#include <helper/types.h>
#include <storage/storage_types_helper.h>
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
struct find_row_functor {
 public:
  typedef void value_type;

  find_row_functor(storage::atable_ptr_t table,
                   storage::c_atable_ptr_t statisticTable,
                   storage::field_t field,
                   storage::value_id_t vid) :
    _table(table),
    _statisticTable(statisticTable),
    _field(field),
    _vid(vid) {}

  template <typename T>
  void operator()() {
    const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(_table->dictionaryAt(_field));
    const T value = dict->getValueForValueId(_vid);

    const auto valueField = _statisticTable->numberOfColumn("value");
    const auto& statisticDict = checked_pointer_cast<storage::BaseDictionary<T>>(_statisticTable->dictionaryAt(valueField));
    const auto& statisticVid = statisticDict->getValueIdForValue(value);

    const auto size = _statisticTable->size();
    for (row = 0; row < size; ++row) {
      if (_statisticTable->getValueId(valueField, row).valueId == statisticVid)
        return;
    }
  }

  size_t row;

 private:
  const storage::atable_ptr_t _table;
  const storage::c_atable_ptr_t _statisticTable;
  const storage::field_t _field;
  const storage::value_id_t _vid;
};
} // namespace

bool TableStatistic::isHot(const std::string& query, value_id_t vid) const {
  if (!isQueryRegistered(query)) {
    return false;
  }

  find_row_functor functor(table(), _statisticTable, field(), vid);
  storage::type_switch<hyrise_basic_types> ts;
  ts(table()->typeOfColumn(field()), functor);

  if (functor.row >= _statisticTable->size())
    return false; // vid not found
  return _statisticTable->getValue<hyrise_int_t>(query, functor.row);
}

bool TableStatistic::isHot(query_t query, value_id_t value) const {
  const auto& qm = QueryManager::instance();
  return isHot(qm.getName(query), value);
}

bool TableStatistic::isQueryRegistered(const std::string& query) const {
  try {
    _statisticTable->numberOfColumn(query);
    return true;
  }
  catch(...) {}
  return false;
}

bool TableStatistic::isQueryRegistered(query_t query) const {
  const auto& qm = QueryManager::instance();
  return isQueryRegistered(qm.getName(query));
}

bool TableStatistic::isVidRegistered(storage::value_id_t vid) const {
  find_row_functor functor(table(), _statisticTable, field(), vid);
  storage::type_switch<hyrise_basic_types> ts;
  ts(table()->typeOfColumn(field()), functor);

  return (functor.row >= _statisticTable->size());
}

namespace {
struct values_do_functor {
  typedef void value_type;

  values_do_functor(storage::atable_ptr_t table, storage::field_t field,
                    storage::c_atable_ptr_t statisticTable, storage::field_t queryField,
		    std::function<void(storage::value_id_t, bool)> func) :
    _table(table),
    _field(field),
    _statisticTable(statisticTable),
    _queryField(queryField),
    _func(func) {}

  template <typename T>
  void operator()() {
    const auto valueField = _statisticTable->numberOfColumn("value");
    const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(_table->dictionaryAt(_field));

    const size_t rowc = _statisticTable->size();
    const auto& hotnessDict = checked_pointer_cast<storage::BaseDictionary<hyrise_int_t>>(_statisticTable->dictionaryAt(_queryField));
    const auto coldValueId = hotnessDict->getValueIdForValue(0);

    for (size_t row = 0; row < rowc; ++row) {
      const T& value = _statisticTable->getValue<T>(valueField, row);
      try {
        const auto valueId = dict->getValueIdForValue(value);
        const auto hotValueId = _statisticTable->getValueId(_queryField, row).valueId;
        _func(valueId, hotValueId != coldValueId);
      }
      catch (...) {}
    }
  }

 private:
  storage::atable_ptr_t _table;
  storage::field_t _field;
  storage::c_atable_ptr_t _statisticTable;
  storage::field_t _queryField;
  std::function<void(storage::value_id_t, bool)> _func;
};
} // namespace

void TableStatistic::valuesDo(query_t query, std::function<void(storage::value_id_t, bool)> func) const {
  const auto& qm = QueryManager::instance();
  if (!qm.exists(qm.getName(query)))
    throw std::runtime_error("QueryID does not exists");
  if (!isQueryRegistered(query))
    return;
  const auto& queryField = _statisticTable->numberOfColumn(qm.getName(query));

  values_do_functor functor(table(), field(), _statisticTable, queryField, func);
  storage::type_switch<hyrise_basic_types> ts;
  ts(table()->typeOfColumn(field()), functor);
}

std::vector<query_t> TableStatistic::queries() const {
  const auto columnc = _statisticTable->columnCount();
  std::vector<query_t> ret;
  const auto& qm = QueryManager::instance();

  for (field_t col = 0; col < columnc; ++col) {
    const auto& columnName = _statisticTable->nameOfColumn(col);
    if (columnName != "value" && qm.exists(columnName))
      ret.push_back(qm.getId(columnName));
  }
  return ret;
}

namespace {

struct collect_table_vids_functor {
  typedef void value_type;

  collect_table_vids_functor(storage::atable_ptr_t table, storage::field_t field, storage::c_atable_ptr_t statisticTable) :
    vids(statisticTable->size()),
    _table(table),
    _field(field),
    _statisticTable(statisticTable) {}

  template <typename T>
  void operator()() {
    const auto& valueField = _statisticTable->numberOfColumn("value");
    const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(_table->dictionaryAt(_field));
    const auto& rowc = _statisticTable->size();

    size_t diff = 0;
    for (size_t row = 0; row < rowc; ++row) {
      try {
        vids.at(row - diff) = dict->getValueIdForValue(_statisticTable->getValue<T>(valueField, row));
      }
      catch (...) { // usually this means the vid does not exist
        ++diff;
      }
    }
    vids.erase(vids.end() - diff, vids.end());
  }

  std::vector<storage::value_id_t> vids;

 private:
  storage::atable_ptr_t _table;
  storage::field_t _field;
  storage::c_atable_ptr_t _statisticTable;
};
} // namespace

std::vector<storage::value_id_t> TableStatistic::vids() const {
  collect_table_vids_functor func(table(), field(), _statisticTable);
  storage::type_switch<hyrise_basic_types> ts;
  ts(table()->typeOfColumn(field()), func);
  return func.vids;
}

} } // namespace hyrise::access

