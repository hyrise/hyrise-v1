// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {
namespace aging {

class EqualExpression : public SelectExpression {
public:
  virtual ~EqualExpression() {}

  virtual std::unique_ptr<AbstractExpression> expression();

  static std::unique_ptr<EqualExpression> parse(const Json::Value& data);

private:
  EqualExpression(const std::string& table, const std::string& field);

  const std::string _table;
  const std::string _field;
};

} } } // namespace aging::hyrise::access

