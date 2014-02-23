// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>
#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {

class RegisterQuery : public PlanOperation {
public:
  RegisterQuery(const std::string& name);

  void executePlanOperation();

  void selectExpression(const std::shared_ptr<aging::SelectExpression>& select);

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  const std::string _name;
  std::shared_ptr<aging::SelectExpression> _select;
};

} } // namespace hyrise::access

