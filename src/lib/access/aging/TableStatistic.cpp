// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableStatistic.h"

#include <storage/AbstractTable.h>

namespace hyrise {
namespace access {

TableStatistic::TableStatistic(storage::atable_ptr_t table) :
    _table(table) {}

TableStatistic::~TableStatistic() {}

void TableStatistic::addStatisticTable(const std::string& field, storage::atable_ptr_t table, bool overRide) {
  const auto fieldId = _table->numberOfColumn(field);
  if (!overRide && _statisticTables.find(fieldId) != _statisticTables.end())
    return;
  
  //TODO implement adding
}

bool TableStatistic::isHot(query_id_t query, field_t field, value_id_t value) {
  //TODO implement
  return false;
}

bool TableStatistic::isRegistered(query_id_t query, field_t field) const {
  //TODO implement
  return false;
}

} } // namespace hyrise::access

