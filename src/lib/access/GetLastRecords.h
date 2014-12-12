// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class GetLastRecords : public PlanOperation {
 public:
  size_t _records;
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
};
}
}
