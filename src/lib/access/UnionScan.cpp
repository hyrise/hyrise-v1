#include <access/UnionScan.h>
#include <storage/HorizontalTable.h>

#include "QueryParser.h"

bool UnionScan::is_registered = QueryParser::registerPlanOperation<UnionScan>();

void UnionScan::executePlanOperation() {
  addResult(std::make_shared<const HorizontalTable>(input.getTables()));
}
