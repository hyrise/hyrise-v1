// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MultiplyRefField.h"

#include "access/BasicParser.h"
#include "access/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<MultiplyRefField>("MultiplyRefField");
}

void MultiplyRefField::executePlanOperation() {
  switch(getInputTable()->typeOfColumn(_field_definition[1])) {
    case IntegerType:
      return executeMultiply<storage::hyrise_int_t, IntegerType>();
    case FloatType:
      return executeMultiply<storage::hyrise_float_t, FloatType>();
    default:
      throw std::runtime_error("The data type of the given field is not supported in MultiplyRefField");
  }
}

std::shared_ptr<_PlanOperation> MultiplyRefField::parse(Json::Value &data) {
  std::shared_ptr<MultiplyRefField> s = BasicParser<MultiplyRefField>::parse(data);
  return s;
}

const std::string MultiplyRefField::vname() {
  return "MultiplyRefField";
}

}
}
