// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AndExpression.h"

#include <iostream>

#include <helper/make_unique.h>

namespace hyrise {
namespace access {
namespace aging {

std::unique_ptr<AbstractExpression> AndExpression::expression() {
  //TODO
}

std::unique_ptr<AndExpression> AndExpression::parse(const Json::Value& data) {
  std::vector<std::unique_ptr<SelectExpression>> subs;
  if (data.isMember("sub")) {
    const auto sub = data["sub"];
    std::cout << ">>>>>>>>>>>>>>>>>" << sub.size() << std::endl;
    for (unsigned i = 0; i < sub.size(); ++i)
      subs.push_back(SelectExpression::parse(sub[i]));
  }
  
  return std::unique_ptr<AndExpression>(new AndExpression(std::move(subs)));
}

AndExpression::AndExpression(std::vector<std::unique_ptr<SelectExpression>>&& subExpressions) :
    _subExpressions(std::move(subExpressions)) {
  std::cout << "creating AndExpression with " << _subExpressions.size() << " subexpressions" << std::endl;
}

} } } // namespace aging::hyrise::access

