// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include <storage/AbstractResource.h>

namespace hyrise {
namespace storage {

class AbstractStatistic : public AbstractResource {
public:
  AbstractStatistic(atable_ptr_t table, field_t field);
  virtual ~AbstractStatistic();

  virtual bool isHot(access::query_t query, value_id_t value) const = 0;
  virtual bool isQueryRegistered(access::query_t query) const = 0;
  virtual bool isValueRegistered(value_id_t vid) const = 0;

  virtual void valuesDo(access::query_t query, std::function<void(value_id_t, bool)> func) const = 0;
  virtual void valuesDo(std::function<void(access::query_t, value_id_t, bool)> func) const;

  virtual std::vector<access::query_t> queries() const = 0;
  virtual std::vector<value_id_t> vids() const = 0;

  atable_ptr_t table() const;
  field_t field() const;

private:
  const std::weak_ptr<AbstractTable> _table;
  const field_t _field;
};

} } // namespace hyrise::storage

