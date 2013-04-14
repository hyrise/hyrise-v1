// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/NoOp.h"

#include "access/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<NoOp>("NoOp");
}

NoOp::~NoOp() {
}

void NoOp::executePlanOperation() {
}

std::shared_ptr<_PlanOperation> NoOp::parse(Json::Value &data) {
  return std::make_shared<NoOp>();
}

const std::string NoOp::vname() {
  return "NoOp";
}

}
}
