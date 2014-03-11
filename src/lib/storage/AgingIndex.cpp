// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AgingIndex.h>

#include <iostream>

#include <storage/storage_types_helper.h>
#include <storage/AbstractStatistic.h>

namespace hyrise {
namespace storage {

AgingIndex::AgingIndex(const c_astat_ptr_t& statistic) :
    _statistic(statistic) {}

AgingIndex::~AgingIndex() {}

void AgingIndex::shrink() { /*no shrinking*/ }

bool AgingIndex::isVidRegistered(value_id_t vid) const {
  return _statistic->isVidRegistered(vid);
}

bool AgingIndex::isHot(access::query_t query, value_id_t vid) const {
  return _statistic->isHot(query, vid);
}

atable_ptr_t AgingIndex::table() {
  return _statistic->table();
}

field_t AgingIndex::field() {
  return _statistic->field();
}

} } // namespace hyrise::storage

