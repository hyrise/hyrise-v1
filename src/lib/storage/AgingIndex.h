// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <unordered_map>

#include <helper/types.h>
#include <storage/AbstractIndex.h>
#include <storage/AbstractTable.h>

namespace hyrise {
namespace storage {

class AgingIndex : public AbstractIndex {
public:
  AgingIndex(const atable_ptr_t& table, const c_astat_ptr_t& statistic);

  virtual ~AgingIndex();
  virtual void shrink();

  /*struct param_t {
    field_t field;
    value_id_t vid;
  };*/
  bool isHot(access::query_t query, value_id_t vid) const;

  atable_ptr_t table();
  field_t field();

private:
  const std::weak_ptr<AbstractTable> _table;
  const c_astat_ptr_t _statistic;
};

} } // namespace hyrise::storage


