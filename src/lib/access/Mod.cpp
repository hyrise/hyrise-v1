// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/Mod.h"

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"
#include "storage/TableBuilder.h"

#include <cmath>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<Mod>("Mod");
}

void Mod::executePlanOperation() {

  auto in = std::const_pointer_cast<storage::AbstractTable>(input.getTable(0));

  storage::TableBuilder::param_list list;

  if (getVType() == 0) {
    list.append().set_type("INTEGER").set_name(getColName());
  } else {
    list.append().set_type("FLOAT").set_name(getColName());
  }

  auto resultTable = storage::TableBuilder::build(list);

  resultTable->resize(in->size());

  if (getVType() == 0) {
    for (size_t row = 0; row < in->size(); row++) {
      resultTable->setValue<hyrise_int_t>(0, row, in->getValue<int>(_field_definition[0], row) % int(getDivisor()));
    }
  } else {
    for (size_t row = 0; row < in->size(); row++) {
      resultTable->setValue<hyrise_float_t>(
          0, row, std::fmod(in->getValue<float>(_field_definition[0], row), getDivisor()));
    }
  }

  addResult(std::make_shared<storage::MutableVerticalTable>(std::vector<storage::atable_ptr_t>{in, resultTable}));
}

std::shared_ptr<PlanOperation> Mod::parse(const Json::Value& data) {
  std::shared_ptr<Mod> instance = BasicParser<Mod>::parse(data);

  if (!data.isMember("fields") || data["fields"].size() != 1) {
    throw std::runtime_error("Exactly one field in 'fields' required");
  }

  if (data.isMember("divisor")) {
    instance->setDivisor(data["divisor"].asFloat());
  } else {
    throw std::runtime_error("'divisor' is missing");
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

void Mod::setDivisor(const float& divisor) {
  if (divisor == 0.0f) {
    throw std::runtime_error("'divisor' connot be zero.");
  }
  _divisor = divisor;
}

void Mod::setVType(const int& vtype) { _vtype = vtype; }

void Mod::setColName(const std::string& colName) { _colName = colName; }

float Mod::getDivisor() const { return _divisor; }

int Mod::getVType() const { return _vtype; }

std::string Mod::getColName() const { return _colName; }
}
}
