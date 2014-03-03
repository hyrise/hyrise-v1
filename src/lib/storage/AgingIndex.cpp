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

bool AgingIndex::isHot(access::query_t query, value_id_t vid) const {
  return _statistic->isHot(query, vid);
}

atable_ptr_t AgingIndex::table() {
  return _table.lock();
}

field_t AgingIndex::field() {
  return _statistic->field();
}

} } // namespace hyrise::storage

