#include "ProjectionScan.h"

#include "storage/storage_types.h"
#include "storage/PointerCalculator.h"
#include "storage/PointerCalculatorFactory.h"

#include "access/QueryParser.h"
#include "access/BasicParser.h"

// Register plan operation for Parsing
bool ProjectionScan::is_registered = QueryParser::registerPlanOperation<ProjectionScan>();

std::shared_ptr<_PlanOperation> ProjectionScan::parse(Json::Value &data) {
  std::shared_ptr<_PlanOperation> p = BasicParser<ProjectionScan>::parse(data);
  return p;
}

ProjectionScan::~ProjectionScan() {
}

void ProjectionScan::setupPlanOperation() {
  computeDeferredIndexes();
}

void ProjectionScan::executePlanOperation() {
  _limit = _limit == 0 ? input.getTable(0)->size() : _limit;

  _limit = _limit > input.getTable(0)->size() ? input.getTable(0)->size() : _limit;

  pos_list_t *pos_list = nullptr;

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

