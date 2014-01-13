// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include <storage/AgingIndex.h>

namespace hyrise {
namespace access {

class AbstractStatistic {
public:
  AbstractStatistic(storage::atable_ptr_t table);
  virtual ~AbstractStatistic();

  virtual bool isHot(query_t query, storage::field_t field, storage::value_id_t value) = 0;
  virtual bool isRegistered(query_t query, storage::field_t field) const = 0;

  std::shared_ptr<storage::AgingIndex> createAgingIndex() const;

  friend class storage::AgingIndex;

protected:
  virtual void valuesDo(std::function<void(query_t, storage::field_t, storage::value_id_t, bool)> func) const = 0;
  storage::atable_ptr_t table() const;

private:
  const storage::atable_ptr_t _table;
};

} } // namespace hyrise::access

