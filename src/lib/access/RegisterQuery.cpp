// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/RegisterQuery.h"

#include <iostream>

#include "access/system/QueryParser.h"
#include "access/system/QueryManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<RegisterQuery>("RegisterQuery");
}

void RegisterQuery::executePlanOperation() {
  std::cout << "====================register query" << std::endl;
  auto& qm = QueryManager::instance();

  output = input; // don't stand in the way of calculation, pass everything on
}

std::shared_ptr<PlanOperation> RegisterQuery::parse(const Json::Value &data) {
  std::shared_ptr<RegisterQuery> rq = std::make_shared<RegisterQuery>();

  /*if (data.isMember("materializing"))
    pop->setProducesPositions(!data["materializing"].asBool());

  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }
  pop->setPredicate(buildExpression(data["predicates"]));

  if (data.isMember("ofDelta")) {
    pop->_ofDelta = data["ofDelta"].asBool();
  }*/

  return rq;
}

} } // namespace hyrise::access

