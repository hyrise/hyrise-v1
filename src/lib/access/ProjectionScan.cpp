// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ProjectionScan.h"

#include "access/QueryParser.h"
#include "access/BasicParser.h"

#include "storage/storage_types.h"
#include "storage/PointerCalculator.h"
#include "storage/PointerCalculatorFactory.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<ProjectionScan>("ProjectionScan");
}

ProjectionScan::ProjectionScan() {
}

ProjectionScan::ProjectionScan(storage::field_list_t *fields): _PlanOperation() {
  setFields(fields);
}

ProjectionScan::~ProjectionScan() {
}

void ProjectionScan::setupPlanOperation() {
  computeDeferredIndexes();
}

void ProjectionScan::executePlanOperation() {
  _limit = _limit == 0 ? input.getTable(0)->size() : _limit;
  _limit = _limit > input.getTable(0)->size() ? input.getTable(0)->size() : _limit;

  storage::pos_list_t *pos_list = nullptr;

  if (_limit != input.getTable(0)->size()) {
    pos_list = new pos_list_t();

    for (size_t i = 0; i < _limit; i++) {
      pos_list->push_back(i);
    }
  }

  // copy the field definition
  std::vector<field_t> *tmp_fd = new std::vector<field_t>(_field_definition);
  addResult(PointerCalculatorFactory::createPointerCalculatorNonRef(input.getTable(0), tmp_fd, pos_list));
}

std::shared_ptr<_PlanOperation> ProjectionScan::parse(Json::Value &data) {
  std::shared_ptr<_PlanOperation> p = BasicParser<ProjectionScan>::parse(data);
  return p;
}

const std::string ProjectionScan::vname() {
  return "ProjectionScan";
}

}
}
