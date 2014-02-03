// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <unordered_map>

#include <storage/AbstractStatistic.h>

namespace hyrise {
namespace access {

class TableStatistic : public storage::AbstractStatistic {
public:
  TableStatistic(storage::atable_ptr_t table, storage::field_t, storage::c_atable_ptr_t statisticTable);
  virtual ~TableStatistic();

  bool isHot(const std::string& query, storage::value_id_t value) const;
  virtual bool isHot(query_t query, storage::value_id_t value) const;

  bool isRegistered(const std::string& query) const;
  virtual bool isRegistered(query_t query) const;

  virtual void valuesDo(std::function<void(query_t, storage::value_id_t, bool)> func) const;

private:
  const storage::c_atable_ptr_t _statisticTable;
};

} } // namespace hyrise::access

