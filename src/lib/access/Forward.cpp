#include "access/Forward.h"

using namespace hyrise::access;

namespace {
  auto forward_reg = QueryParser::registerTrivialPlanOperation<Forward>("Forward");
}

void Forward::executePlanOperation() {
  output = input;
}
