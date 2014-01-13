// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AbstractStatistic.h"

namespace hyrise {
namespace access {

AbstractStatistic::AbstractStatistic(storage::atable_ptr_t table) :
    _table(table) {}

AbstractStatistic::~AbstractStatistic() {}

std::shared_ptr<storage::AgingIndex> AbstractStatistic::createAgingIndex() const {
  const auto agingIndex = std::make_shared<storage::AgingIndex>(table());

  //TODO implement

  return agingIndex;
}

storage::atable_ptr_t AbstractStatistic::table() const {
  return _table;
}

} } // namespace hyrise::access

