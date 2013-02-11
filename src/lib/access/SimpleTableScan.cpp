// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SimpleTableScan.h"

#include <storage/meta_storage.h>
#include <storage/storage_types.h>
#include <storage/PointerCalculator.h>
#include <storage/PointerCalculatorFactory.h>
#include <storage/Table.h>
#include <storage/MutableVerticalTable.h>

#include "pred_buildExpression.h"

bool SimpleTableScan::is_registered = QueryParser::registerPlanOperation<SimpleTableScan>();

void SimpleTableScan::setupPlanOperation() {
  _comparator->walk(input.getTables());
}


std::shared_ptr<_PlanOperation> SimpleTableScan::parse(Json::Value &data) {
  std::shared_ptr<SimpleTableScan> pop = std::make_shared<SimpleTableScan>();

  if (data.isMember("materializing"))
    pop->setProducesPositions(!data["materializing"].asBool());


  // Get the tree from the builder
  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }

  pop->setPredicate(buildExpression(data["predicates"]));

  return pop;
}

void SimpleTableScan::executePlanOperation() {
  pos_list_t *pos_list = nullptr;
  hyrise::storage::atable_ptr_t result_table;

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

  hyrise::storage::atable_ptr_t result;
  // In case we are creating positions copy the pos
  // list
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

