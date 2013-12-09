// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleTableScan.h"

#include "access/expressions/pred_buildExpression.h"

#include "storage/Store.h"
#include "storage/PointerCalculator.h"

#include "helper/checked_cast.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SimpleTableScan>("SimpleTableScan");
}

SimpleTableScan::SimpleTableScan(): _comparator(nullptr) {
}

SimpleTableScan::~SimpleTableScan() {
  if (_comparator)
    delete _comparator;
}

void SimpleTableScan::setupPlanOperation() {
  _comparator->walk(input.getTables());
}

void SimpleTableScan::executePositional() {
  auto tbl = input.getTable(0);
  storage::pos_list_t *pos_list = new pos_list_t();


  size_t row = _ofDelta ? checked_pointer_cast<const storage::Store>(tbl)->deltaOffset() : 0;
  for (size_t input_size=tbl->size(); row < input_size; ++row) {
    if ((*_comparator)(row)) {
      pos_list->push_back(row);
    }
  }
  addResult(storage::PointerCalculator::create(tbl, pos_list));
}

void SimpleTableScan::executeMaterialized() {
  auto tbl = input.getTable(0);
  auto result_table = tbl->copy_structure_modifiable();
  size_t target_row = 0;

  size_t row = _ofDelta ? checked_pointer_cast<const storage::Store>(tbl)->deltaOffset() : 0;
  for (size_t input_size=tbl->size();
       row < input_size;
       ++row) {
    if ((*_comparator)(row)) {
        // TODO materializing result set will make the allocation the boundary
      result_table->resize(target_row + 1);
      result_table->copyRowFrom(input.getTable(0),
                                row,
                                target_row++,
                                true /* Copy Value*/,
                                false /* Use Memcpy */);
    }
  }
  addResult(result_table);
}

void SimpleTableScan::executePlanOperation() {
  if (producesPositions) {
    executePositional();
  } else {
    executeMaterialized();
  }
}

std::shared_ptr<PlanOperation> SimpleTableScan::parse(const Json::Value &data) {
  std::shared_ptr<SimpleTableScan> pop = std::make_shared<SimpleTableScan>();

  if (data.isMember("materializing"))
    pop->setProducesPositions(!data["materializing"].asBool());

  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }
  pop->setPredicate(buildExpression(data["predicates"]));

  if (data.isMember("ofDelta")) {
    pop->_ofDelta = data["ofDelta"].asBool();
  }

  return pop;
}

const std::string SimpleTableScan::vname() {
  return "SimpleTableScan";
}

void SimpleTableScan::setPredicate(SimpleExpression *c) {
  _comparator = c;
}

}
}
