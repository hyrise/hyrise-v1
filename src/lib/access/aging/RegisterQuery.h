// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>
#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {

class RegisterQuery : public PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  std::string _name;
  std::shared_ptr<aging::SelectExpression> _select;

  typedef std::vector<std::string> field_list_t;
  std::map<std::string, field_list_t> _fields;
};

} } // namespace hyrise::access

