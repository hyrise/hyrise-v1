// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class CompoundIndexScan : public PlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  void setIndexName(AbstractExpression* c);
};
}
}
