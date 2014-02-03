// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {
namespace aging {

class AndExpression : public SelectExpression {
public:
  virtual ~AndExpression() {}

  virtual std::unique_ptr<AbstractExpression> expression();

  static std::unique_ptr<AndExpression> parse(const Json::Value& data);

private:
  AndExpression(std::vector<std::unique_ptr<SelectExpression>>&& subExpressions);

  std::vector<std::unique_ptr<SelectExpression>> _subExpressions;
};

} } } // namespace aging::hyrise::access

