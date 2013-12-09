// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IntersectPositions.h"
#include "storage/PointerCalculator.h"

#include <algorithm>

namespace hyrise {
namespace access {

namespace { auto _ = QueryParser::registerPlanOperation<IntersectPositions>("IntersectPositions"); }

std::shared_ptr<PlanOperation> IntersectPositions::parse(const Json::Value&) {
  return std::make_shared<IntersectPositions>();
}

void IntersectPositions::executePlanOperation() {
  const auto& tables = input.getTables();
  std::vector<std::shared_ptr<const storage::PointerCalculator>> pcs(tables.size());
  std::transform(begin(tables), end(tables),
                 begin(pcs),
                 [] (decltype(*begin(tables)) table) {
                   return std::dynamic_pointer_cast<const storage::PointerCalculator>(table);
                 });
  if (std::all_of(begin(pcs), end(pcs), [] (decltype(*begin(tables)) pc) { return pc != nullptr; })) {
    addResult(storage::PointerCalculator::intersect_many(begin(pcs), end(pcs)));
  } else {
    throw std::runtime_error(_planOperationName + " is only supported for PointerCalculators (IntersectPositions.cpp)");
  }
}

const std::string IntersectPositions::vname() {
  return "IntersectPositions";
};

}
}
