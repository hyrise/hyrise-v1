// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/AgingCheck.h"

#include <iostream>

#include "access/system/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<AgingCheck>("AgingCheck");
}

void AgingCheck::executePlanOperation() {
  std::cout << "========================executing aging check" << std::endl;

  output = input; // don't stand in the way of calculation, pass everything on
}

std::shared_ptr<PlanOperation> AgingCheck::parse(const Json::Value &data) {
  std::shared_ptr<AgingCheck> ac = std::make_shared<AgingCheck>();

  /*if (data.isMember("materializing"))
    pop->setProducesPositions(!data["materializing"].asBool());

  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }
  pop->setPredicate(buildExpression(data["predicates"]));

  if (data.isMember("ofDelta")) {
    pop->_ofDelta = data["ofDelta"].asBool();
  }*/

  return ac;
}

} } // namespace hyrise::access

