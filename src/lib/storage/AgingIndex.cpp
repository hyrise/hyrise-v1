// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AgingIndex.h>

#include <iostream>

#include <storage/storage_types_helper.h>
#include <storage/AbstractStatistic.h>

namespace hyrise {
namespace storage {

AgingIndex::AgingIndex(const atable_ptr_t& table, const c_astat_ptr_t& statistic) :
    _table(table),
    _statistic(statistic) {}

AgingIndex::~AgingIndex() {}

void AgingIndex::shrink() {
  //TODO ... implement it?
}

void AgingIndex::addForQuery(access::query_t query, const std::vector<field_t>& fields) {
  //TODO ... is this still needed?
}

bool AgingIndex::isHot(access::query_t query, std::vector<param_t> params) const {
  bool hot = true;
  for (const auto& param : params)
    hot &= _statistic->isHot(query, param.field, param.vid);
  return hot;
}

atable_ptr_t AgingIndex::table() {
  return _table.lock();
}

} } // namespace hyrise::storage

