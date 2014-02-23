// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class AgingRun : public PlanOperation {
public:
  AgingRun(const std::string& table);

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

private:
  const std::string _tableName;
};

} } // namespace hyrise::access

