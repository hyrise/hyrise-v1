// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <map>

#include <access/aging/AbstractStatistic.h>

namespace hyrise {
namespace access {

class TableStatistic : public AbstractStatistic {
public:
  TableStatistic(storage::atable_ptr_t table);
  virtual ~TableStatistic();

  //TODO override is a keyword hrng
  void addStatisticTable(const std::string& field, storage::atable_ptr_t table, bool overRide = false);

  virtual bool isHot(query_id_t query, storage::field_t field, storage::value_id_t value);
  virtual bool isRegistered(query_id_t query, storage::field_t field) const;

private:
  const storage::atable_ptr_t _table;
  std::map<storage::field_t, storage::atable_ptr_t> _statisticTables;
};

} } // namespace hyrise::access

