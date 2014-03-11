// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AbstractStatistic.h"

#include <storage/AbstractTable.h>

namespace hyrise {
namespace storage {

AbstractStatistic::AbstractStatistic(atable_ptr_t table, field_t field) :
    _table(table),
    _field(field) {
  if (this->table()->columnCount() <= field)
    throw std::runtime_error("invalid field");
}

AbstractStatistic::~AbstractStatistic() {}

void AbstractStatistic::valuesDo(std::function<void(access::query_t, value_id_t, bool)> func) const {
  const auto& queries = this->queries();
  for (const auto& query : queries)
    valuesDo(query, [&query, &func] (value_id_t vid, bool hot) { func(query, vid, hot); });
}

void AbstractStatistic::table(atable_ptr_t table) {
  _table = table;
}

atable_ptr_t AbstractStatistic::table() const {
  return _table.lock();
}

field_t AbstractStatistic::field() const {
  return _field;
}

} } // namespace hyrise::storage

