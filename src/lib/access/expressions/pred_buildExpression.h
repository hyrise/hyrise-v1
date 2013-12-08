// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "pred_common.h"

namespace hyrise {
namespace access {

class ExpressionBuildError : public std::runtime_error {
 public:
  explicit ExpressionBuildError(const std::string &what): std::runtime_error(what) {}
};

SimpleFieldExpression *buildFieldExpression(PredicateType::type,const Json::Value &);
SimpleExpression *buildExpression(const Json::Value &);

} } // namespace hyrise::access

