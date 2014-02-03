// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "EqualExpression.h"

#include <iostream>

#include <helper/make_unique.h>

namespace hyrise {
namespace access {
namespace aging {

std::unique_ptr<AbstractExpression> EqualExpression::expression() {
  //TODO
}

std::unique_ptr<EqualExpression> EqualExpression::parse(const Json::Value& data) {
  if (!data.isMember("table"))
    throw std::runtime_error("a table needs to be specified for an expression");
  if (!data.isMember("field"))
    throw std::runtime_error("a field needs to be specified for an expression");

  const auto& table = data["table"].asString();
  const auto& field = data["field"].asString();

  return std::unique_ptr<EqualExpression>(new EqualExpression(table, field));
}

EqualExpression::EqualExpression(const std::string& table, const std::string& field) :
    _table(table),
    _field(field) {
  std::cout << "creating EqualExpression [" << table << "." << field << "]" << std::endl;
}

} } } // namespace aging::hyrise::access

