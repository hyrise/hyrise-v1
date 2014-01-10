// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MultiplyRefField.h"

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"
#include "storage/ColumnMetadata.h"
#include "storage/MutableVerticalTable.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<MultiplyRefField>("MultiplyRefField");
}

template<typename T, DataType D>
void MultiplyRefField::executeMultiply() {
  const auto ref = _field_definition[0];
  const auto val = _field_definition[1];

  const auto stop = getInputTable()->size();
  const auto stopInner = getInputTable(1)->size();

  const auto &itab = getInputTable();
  const auto &mulTab = getInputTable(1);

  std::vector<storage::ColumnMetadata> meta {storage::ColumnMetadata("value", D), storage::ColumnMetadata("pos", IntegerType)};
  auto result = std::make_shared<storage::Table>(&meta, nullptr, stop * stopInner, false, false);
  result->resize(stop * stopInner);

  // Pos list for matching Rows
  auto pos = new std::vector<size_t>;

  // Nested Loop for Multiplication
  for(size_t outer=0; outer < stop; ++outer) {
    const auto tmp = itab->getValue<T>(val, outer);
    const auto col = itab->getValue<hyrise_int_t>(ref, outer);

    for(size_t inner=0; inner < stopInner; ++inner) {
      pos->push_back(outer);
      result->setValue<T>(0, stopInner * outer + inner, mulTab->getValue<T>(col, inner) * tmp);
      result->setValue<hyrise_int_t>(1, stopInner * outer + inner, inner);
    }
  }

  auto left = storage::PointerCalculator::create(getInputTable(), pos);

  std::vector<storage::atable_ptr_t> vc({left, result});
  storage::atable_ptr_t tt = std::make_shared<storage::MutableVerticalTable>(vc);
  addResult(tt);
}


void MultiplyRefField::executePlanOperation() {
  switch(getInputTable()->typeOfColumn(_field_definition[1])) {
    case IntegerType:
    case IntegerTypeDelta:
    case IntegerTypeDeltaConcurrent:
      return executeMultiply<storage::hyrise_int_t, IntegerTypeDelta>();
    case FloatType:
    case FloatTypeDelta:
    case FloatTypeDeltaConcurrent:
      return executeMultiply<storage::hyrise_float_t, FloatTypeDelta>();
    default:
      throw std::runtime_error("The data type of the given field is not supported in MultiplyRefField");
  }
}

std::shared_ptr<PlanOperation> MultiplyRefField::parse(const Json::Value &data) {
  std::shared_ptr<MultiplyRefField> s = BasicParser<MultiplyRefField>::parse(data);
  return s;
}

const std::string MultiplyRefField::vname() {
  return "MultiplyRefField";
}

}
}
