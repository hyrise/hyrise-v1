// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "storage/storage_types.h"
#include "helper/types.h"
#include "access/expressions/AbstractExpression.h"

namespace hyrise {
namespace access {

class SimpleExpression : public access::AbstractExpression {
 public:
  virtual void walk(const std::vector<storage::c_atable_ptr_t> &l) = 0;

  virtual pos_list_t* match(const size_t start, const size_t stop) {
    auto pl = new pos_list_t;
    for(size_t row=0; row < stop; ++row) {
      if (operator()(row)) {
        pl->push_back(row);
      }
    }
    return pl;
  }

  inline virtual bool operator()(size_t row) {
    throw std::runtime_error("Cannot call base class");
  }
};

} } // namespace hyrise::access

