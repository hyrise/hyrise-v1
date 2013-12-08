// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/Distinct.h"

#include <unordered_map>

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "helper/types.h"

#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<Distinct>("Distinct");
}

Distinct::~Distinct() {
}

// Executing this on a store with delta results in undefined behavior
// Execution with horizontal tables results in undefined behavior
void Distinct::executePlanOperation() {
  // Map to cache values
  std::unordered_map<storage::value_id_t, storage::pos_t> map;
  auto distinct = _field_definition[0];

  // iterate over all rows
  const auto &in = input.getTable(0);
  uint64_t numRows = in->size();
  ValueId val;

  // Build distinct value list
  for (uint64_t i = 0; i < numRows; ++i) {
    val = in->getValueId(distinct, i);
    if (map.count(val.valueId) == 0)
      map[val.valueId] = i;
  }

  //Build result list
  storage::atable_ptr_t result;
  auto pos = new storage::pos_list_t;
  for (const auto &e : map)
    pos->push_back(e.second);

  // Return pointer calculator
  addResult(storage::PointerCalculator::create(input.getTable(0), pos));
}

std::shared_ptr<PlanOperation> Distinct::parse(const Json::Value &data) {
  std::shared_ptr<Distinct> s = BasicParser<Distinct>::parse(data);
  return s;
}

const std::string Distinct::vname() {
  return "Distinct";
}

}
}
