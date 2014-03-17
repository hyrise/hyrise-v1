// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "pred_SimpleExpression.h"

namespace hyrise {
namespace access {

class FalseExpression : public SimpleExpression {
 public:
  virtual void walk(const std::vector<storage::c_atable_ptr_t>& l) {}
  inline virtual bool operator()(size_t row) { return false; }
};

} }  // namespace hyrise::access

