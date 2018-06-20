// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/expressions/ExpressionLibrary.h"
#include <iostream>

namespace hyrise {
namespace access {

bool ExpressionLibrary::add(const std::string& name, std::unique_ptr<Expression>(*function)(const Json::Value&)) {
  Expressions[name] = function;

  return true;
}

std::unique_ptr<Expression> ExpressionLibrary::dispatchAndParse(const Json::Value& data) {
  return Expressions.at(data["expression"].asString())(data);
}
}
}