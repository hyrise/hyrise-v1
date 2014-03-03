// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class RegisterTableStatistic : public PlanOperation {
public:
  RegisterTableStatistic(const std::string& table, const std::string& field);

  void executePlanOperation();

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  const std::string _table;
  const std::string _field;
};

} } // namespace hyrise::access

