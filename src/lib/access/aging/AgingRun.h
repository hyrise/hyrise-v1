// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class AgingRun : public PlanOperation {
public:
  ~AgingRun();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  std::string _tableName;
  storage::astat_ptr_t _statistic;
};

} } // namespace hyrise::access

