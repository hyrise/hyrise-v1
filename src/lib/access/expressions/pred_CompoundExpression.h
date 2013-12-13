// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "pred_common.h"

namespace hyrise {
namespace access {

class CompoundExpression : public SimpleExpression {
 private:

  ExpressionType type;

 public:

  SimpleExpression *lhs;
  SimpleExpression *rhs;

  // If the compound expression only has one child
  bool one_leg;

  explicit CompoundExpression(ExpressionType t):
      type(t), lhs(nullptr), rhs(nullptr), one_leg(t == NOT)
  {}

  CompoundExpression(SimpleExpression *_lhs, SimpleExpression *_rhs, ExpressionType _type) :
      SimpleExpression(), type(_type), lhs(_lhs), rhs(_rhs), one_leg(_type == NOT)
  {}

  virtual ~CompoundExpression() {
    delete lhs;

    if (!one_leg) {
      delete rhs;
    }
  }

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &l) {
    lhs->walk(l);

    if (!one_leg) {
      rhs->walk(l);
    }
  }

  inline virtual bool operator()(size_t row) {
    switch (type) {
      case AND:
        return (*lhs)(row) && (*rhs)(row);
        break;

      case OR:
        return (*lhs)(row) || (*rhs)(row);
        break;

      case NOT:
        return !(*lhs)(row);
        break;

      default:
        throw std::runtime_error("Unknown Expression Type");
        break;
    }
  }

  inline void add(SimpleExpression *e) {
    if (!lhs) lhs = e;
    else if (!rhs) rhs = e;
    else throw std::runtime_error("full");
  }

  inline bool isSetup() {
    return ((one_leg) && (lhs != nullptr)) || ((rhs != nullptr) && (lhs != nullptr));
  }
};

} } // namespace hyrise::access

