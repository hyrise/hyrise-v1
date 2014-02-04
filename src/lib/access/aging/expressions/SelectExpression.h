// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <jsoncpp/json.h>

#include <access/expressions/AbstractExpression.h>

namespace hyrise {
namespace access {
namespace aging {

class SelectExpression {
public:
  virtual ~SelectExpression() {}

  virtual std::unique_ptr<AbstractExpression> expression() const = 0;
  virtual void verify() const = 0;

  static std::unique_ptr<SelectExpression> parse(const Json::Value& data);
};

} } } // namespace aging::hyrise::access

