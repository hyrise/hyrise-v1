// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ProjectionScan.h"

#include "access/system/QueryParser.h"
#include "access/system/BasicParser.h"

#include "storage/storage_types.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<ProjectionScan>("ProjectionScan");
}

void ProjectionScan::setupPlanOperation() {
  computeDeferredIndexes();
}

void ProjectionScan::executePlanOperation() {
  storage::pos_list_t *pos_list = nullptr;

  // copy the field definition
  std::vector<field_t> *tmp_fd = new std::vector<field_t>(_field_definition);
  addResult(PointerCalculator::create(input.getTable(0), pos_list, tmp_fd));
}

std::shared_ptr<PlanOperation> ProjectionScan::parse(Json::Value &data) {
  std::shared_ptr<PlanOperation> p = BasicParser<ProjectionScan>::parse(data);
  return p;
}

const std::string ProjectionScan::vname() {
  return "ProjectionScan";
}

}
}
