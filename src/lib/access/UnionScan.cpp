// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/UnionScan.h"

#include "access/QueryParser.h"

#include "storage/HorizontalTable.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<UnionScan>("UnionScan");
}

UnionScan::~UnionScan() {
}

void UnionScan::executePlanOperation() {
  addResult(std::make_shared<const HorizontalTable>(input.getTables()));
}

std::shared_ptr<_PlanOperation> UnionScan::parse(Json::Value &data) {
  return std::make_shared<UnionScan>();
}

const std::string UnionScan::vname() {
  return "UnionScan";
}

}
}
