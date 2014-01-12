// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>

namespace hyrise {
namespace access {

class AbstractStatistic {
public:
  virtual ~AbstractStatistic() {}

  virtual bool isHot(query_id_t query, storage::field_t field, storage::value_id_t value) = 0;
  virtual bool isRegistered(query_id_t query, storage::field_t field) const = 0;
};

} } // namespace hyrise::access

