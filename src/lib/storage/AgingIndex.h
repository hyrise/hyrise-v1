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

  void addForQuery(access::query_t query, const std::vector<field_t>& fields);

  struct param_t {
    field_t field;
    value_id_t vid;
  };
  bool isHot(access::query_t query, std::vector<param_t> params) const;

  atable_ptr_t table();

private:
  const std::weak_ptr<AbstractTable> _table;
  const c_astat_ptr_t _statistic;
};

} } // namespace hyrise::storage


