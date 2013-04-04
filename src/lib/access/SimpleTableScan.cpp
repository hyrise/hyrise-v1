// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleTableScan.h"

#include "access/pred_buildExpression.h"

#include "storage/PointerCalculator.h"
#include "storage/PointerCalculatorFactory.h"

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

void SimpleTableScan::executePlanOperation() {
  storage::pos_list_t *pos_list = nullptr;
  storage::atable_ptr_t result_table;

  if (producesPositions) {
    pos_list = new pos_list_t();
  } else {
    result_table = input.getTable(0)->copy_structure_modifiable();
  }

  // Iterate over the data
  size_t input_size = input.getTable(0)->size();
  size_t target_row = 0;

  for (size_t row = 0; row < input_size; ++row) {
    // Execute the predicate on the list
    if ((*_comparator)(row)) {
      if (producesPositions) {
        pos_list->push_back(row);
      } else {
        // TODO materializing result set will make the allocation the boundary
        result_table->resize(target_row + 1);
        result_table->copyRowFrom(input.getTable(0),
                                  row,
                                  target_row++,
                                  true /* Copy Value*/,
                                  false /* Use Memcpy */);
      }
    }
  }

  storage::atable_ptr_t result;

  // In case we are creating positions copy the pos_list
  if (producesPositions) {
    if (!pos_list->empty()) {
      result = PointerCalculatorFactory::createPointerCalculatorNonRef(input.getTable(), nullptr, pos_list);
    } else {
      result = input.getTable()->copy_structure_modifiable();
    }
  } else {
    result = result_table;
  }
  addResult(result);
}

std::shared_ptr<_PlanOperation> SimpleTableScan::parse(Json::Value &data) {
  std::shared_ptr<SimpleTableScan> pop = std::make_shared<SimpleTableScan>();

  if (data.isMember("materializing"))
    pop->setProducesPositions(!data["materializing"].asBool());

  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }
  pop->setPredicate(buildExpression(data["predicates"]));

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
