// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/Division.h"

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"
#include "storage/TableBuilder.h"

#include <cmath>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<Division>("Division");
}

void Division::executePlanOperation() {

  auto in = std::const_pointer_cast<storage::AbstractTable>(input.getTable(0));

  storage::TableBuilder::param_list list;

  if (getVType() == 0) {
    list.append().set_type("INTEGER").set_name(getColName());
  } else {
    list.append().set_type("FLOAT").set_name(getColName());
  }

  auto resultTable = storage::TableBuilder::build(list);

  resultTable->resize(in->size());

  for (size_t row = 0; row < in->size(); row++) {
    float divident, divisor;
    if (in->typeOfColumn(_field_definition[0]) == IntegerType)
      divident = (float)in->getValue<hyrise_int_t>(_field_definition[0], row);
    else
      divident = in->getValue<hyrise_float_t>(_field_definition[0], row);

    if (in->typeOfColumn(_field_definition[1]) == IntegerType)
      divisor = (float)in->getValue<hyrise_int_t>(_field_definition[1], row);
    else
      divisor = in->getValue<hyrise_float_t>(_field_definition[1], row);

    if (divisor == 0.0) {
      throw std::runtime_error("Division with zero not allowed!");
    }

    if (getVType() == 0) {
      resultTable->setValue<hyrise_int_t>(0, row, (hyrise_int_t)(divident / divisor));
    } else {
      resultTable->setValue<hyrise_float_t>(0, row, divident / divisor);
    }
  }

  addResult(std::make_shared<storage::MutableVerticalTable>(std::vector<storage::atable_ptr_t>{in, resultTable}));
}

std::shared_ptr<PlanOperation> Division::parse(const Json::Value& data) {
  std::shared_ptr<Division> instance = BasicParser<Division>::parse(data);

  if (!data.isMember("fields") || data["fields"].size() != 2) {
    throw std::runtime_error("Exactly two fields have to be defined in \"fields\"");
  }

  if (data.isMember("as")) {
    instance->setColName(data["as"].asString());
  } else {
    throw std::runtime_error("'as' has to be defined");
  }

  if (data.isMember("vtype")) {
    instance->setVType(data["vtype"].asInt());
    if (instance->getVType() != 1 && instance->getVType() != 0) {
      throw std::runtime_error("vtype has to be 0 or 1 for int or float");
    }
  } else {
    throw std::runtime_error("'vtype' has to be defined (0 or 1 for int or float)");
  }

  return instance;
}

void Division::setVType(const int& vtype) { _vtype = vtype; }

void Division::setColName(const std::string& colName) { _colName = colName; }

int Division::getVType() const { return _vtype; }

std::string Division::getColName() const { return _colName; }
}
}
