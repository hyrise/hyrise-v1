// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_BUILDEXPRESSION_H_
#define SRC_LIB_ACCESS_PRED_BUILDEXPRESSION_H_

#include "pred_common.h"

class ExpressionBuildError : public std::runtime_error {
 public:
  explicit ExpressionBuildError(const std::string &what): std::runtime_error(what) {}
};

SimpleFieldExpression *buildFieldExpression(PredicateType::type,const Json::Value &);
SimpleExpression *buildExpression(const Json::Value &);

#endif  // SRC_LIB_ACCESS_PRED_BUILDEXPRESSION_H_
