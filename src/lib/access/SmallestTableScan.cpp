// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SmallestTableScan.h"

#include "access/system/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SmallestTableScan>("SmallestTableScan");
}

SmallestTableScan::~SmallestTableScan() {
}

void SmallestTableScan::setupPlanOperation() {
  computeDeferredIndexes();
}

void SmallestTableScan::executePlanOperation() {
  auto smallestTable = getInputTable(0);
  for (size_t i = 1; i < input.numberOfTables(); ++i) {
    auto nextTable = getInputTable(i);
    if (nextTable->size() < smallestTable->size()) {
      smallestTable = nextTable;
    }
  }
  addResult(smallestTable);
}

std::shared_ptr<PlanOperation> SmallestTableScan::parse(const Json::Value &data) {
  return std::make_shared<SmallestTableScan>();
}

const std::string SmallestTableScan::vname() {
  return "SmallestTableScan";
}

}
}
