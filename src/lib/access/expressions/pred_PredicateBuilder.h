// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once
#include <stack>

#include "pred_common.h"
#include "pred_CompoundExpression.h"

namespace hyrise {
namespace access {

/**
 * @brief Creates the predicate tree required for Selections
 *
 * All predicates both SimpleFieldExpression and CompoundExpression
 * have to be added in the correct execution order using prefix
 * notation.
 */
class PredicateBuilder {
  std::stack<CompoundExpression *> previous;
  SimpleExpression *root;

 public:
  PredicateBuilder();
  virtual ~PredicateBuilder();
  void add(SimpleFieldExpression *exp);
  void add(CompoundExpression *exp);
  SimpleExpression *build();
};

} } // namespace hyrise::access

