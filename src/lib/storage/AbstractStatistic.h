// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include <storage/AbstractResource.h>

namespace hyrise {
namespace storage {

class AbstractStatistic : public AbstractResource {
public:
  AbstractStatistic(atable_ptr_t table);
  virtual ~AbstractStatistic();

  virtual bool isHot(access::query_t query, field_t field, value_id_t value) const = 0;
  virtual bool isRegistered(access::query_t query, field_t field) const = 0;

  virtual void valuesDo(std::function<void(access::query_t, field_t, value_id_t, bool)> func) const = 0;

protected:
  atable_ptr_t table() const;

private:
  const atable_ptr_t _table;
};

} } // namespace hyrise::storage

