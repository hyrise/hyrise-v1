#include "SmallestTableScan.h"
#include "QueryParser.h"

bool SmallestTableScan::is_registered = QueryParser::registerPlanOperation<SmallestTableScan>();

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

std::shared_ptr<_PlanOperation> SmallestTableScan::parse(Json::Value &data) {
  return std::make_shared<SmallestTableScan>();
}

