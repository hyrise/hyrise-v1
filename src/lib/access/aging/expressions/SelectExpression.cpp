// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "SelectExpression.h"

#include <access/aging/expressions/AndExpression.h>
#include <access/aging/expressions/EqualExpression.h>

namespace hyrise {
namespace access {
namespace aging {

std::unique_ptr<SelectExpression> SelectExpression::parse(const Json::Value& data) {
  if (!data.isMember("operation"))
    throw std::runtime_error("an operation needs to be specified");

  const auto op = data["operation"].asString();

  if (op == "AND")     return AndExpression::parse(data);
  else if (op == "EQ") return EqualExpression::parse(data);
  else throw std::runtime_error("unsupported operation: \"" + op + "\"");
}

} } } // namespace aging::hyrise::access

