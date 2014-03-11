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

  bool isQueryRegistered(const std::string& query) const;
  virtual bool isQueryRegistered(query_t query) const;
  bool isVidRegistered(const std::string& value) const;
  virtual bool isVidRegistered(storage::value_id_t vid) const;

  virtual void valuesDo(query_t query, std::function<void(storage::value_id_t, bool)> func) const;

  virtual std::vector<query_t> queries() const;
  virtual std::vector<storage::value_id_t> vids() const;

private:
  const storage::c_atable_ptr_t _statisticTable;
};

} } // namespace hyrise::access

