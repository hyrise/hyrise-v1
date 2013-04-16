// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/Barrier.h"

#include "access/BasicParser.h"
#include "access/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<Barrier>("Barrier");
}

void Barrier::executePlanOperation() {
  for(size_t i=0; i < _field_definition.size(); ++i)
    addResult(getInputTable(i));
}

std::shared_ptr<_PlanOperation> Barrier::parse(Json::Value &data) {
  return BasicParser<Barrier>::parse(data);
}

const std::string Barrier::vname() {
  return "Barrier";
}

}
}
