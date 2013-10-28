// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/NoOp.h"
#include "access/system/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerTrivialPlanOperation<NoOp>("NoOp");
}

void NoOp::executePlanOperation() {}

}}
