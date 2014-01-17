// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <unordered_map>

#include <storage/AbstractStatistic.h>

namespace hyrise {
namespace access {

class TableStatistic : public storage::AbstractStatistic {
public:
  TableStatistic(storage::atable_ptr_t table);
  virtual ~TableStatistic();

  //TODO override is a keyword hrng
  void addStatisticTable(const std::string& field, storage::c_atable_ptr_t table, bool overRide = false);
  void addStatisticTable(storage::field_t field, storage::c_atable_ptr_t table, bool overRide = false);

  bool isHot(const std::string& query, storage::field_t field, storage::value_id_t value) const;
  virtual bool isHot(query_t query, storage::field_t field, storage::value_id_t value) const;

  bool isRegistered(const std::string& query, storage::field_t field) const;
  virtual bool isRegistered(query_t query, storage::field_t field) const;

  virtual void valuesDo(std::function<void(query_t, storage::field_t, storage::value_id_t, bool)> func) const;

private:
  std::unordered_map<storage::field_t, storage::c_atable_ptr_t> _tables;
};

} } // namespace hyrise::access

