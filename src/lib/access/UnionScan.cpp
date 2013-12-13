// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/UnionScan.h"

#include <algorithm>

#include "storage/HorizontalTable.h"
#include "storage/PointerCalculator.h"

namespace hyrise { namespace access {

namespace {
auto _1 = QueryParser::registerTrivialPlanOperation<UnionScan>("UnionScan");
auto _2 = QueryParser::registerTrivialPlanOperation<UnionScan>("Union");
}

void UnionScan::executePlanOperation() {
  const auto& tables = input.getTables();
  std::vector<std::shared_ptr<const storage::PointerCalculator>> pcs(tables.size());
  std::transform(begin(tables), end(tables),
                 begin(pcs),
                 [] (decltype(*begin(tables)) table) {
                   return std::dynamic_pointer_cast<const storage::PointerCalculator>(table);
                 });
  if (std::all_of(begin(pcs), end(pcs), [] (decltype(*begin(tables)) pc) { return pc != nullptr; })) {
    addResult(storage::PointerCalculator::unite_many(begin(pcs), end(pcs)));
  } else {
    throw std::runtime_error(_planOperationName + " is only supported for PointerCalculators (UnionScan.cpp)");
    // We need to implement duplicate elimination here to make this a true union
    // If you tripped over this error, you probably meant to use UnionAll.
    //addResult(std::make_shared<const HorizontalTable>(tables));
  }
}

}}
