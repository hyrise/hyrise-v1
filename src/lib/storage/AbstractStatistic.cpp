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

atable_ptr_t AbstractStatistic::table() const {
  return _table.lock();
}

field_t AbstractStatistic::field() const {
  return _field;
}

} } // namespace hyrise::storage

