// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AbstractStatistic.h"

namespace hyrise {
namespace storage {

AbstractStatistic::AbstractStatistic(atable_ptr_t table) :
    _table(table) {}

AbstractStatistic::~AbstractStatistic() {}

atable_ptr_t AbstractStatistic::table() const {
  return _table;
}

} } // namespace hyrise::storage

