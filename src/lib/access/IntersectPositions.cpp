// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IntersectPositions.h"

#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

namespace { auto _ = QueryParser::registerPlanOperation<IntersectPositions>("IntersectPositions"); }

std::shared_ptr<_PlanOperation> IntersectPositions::parse(Json::Value&) {
  return std::make_shared<IntersectPositions>();
}

void IntersectPositions::executePlanOperation() {
  const auto& pc1 = std::dynamic_pointer_cast<const PointerCalculator>(getInputTable(0));
  const auto& pc2 = std::dynamic_pointer_cast<const PointerCalculator>(getInputTable(1));

  if (pc1 == nullptr) { throw std::runtime_error("Passed input 0 is not a PC!"); }
  if (pc2 == nullptr) { throw std::runtime_error("Passed input 1 is not a PC!"); }

  auto pc3 = pc1->intersect(pc2);

  addResult(pc3);
}

const std::string IntersectPositions::vname() {
  return "IntersectPositions";
};

}
}
