// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SubString.h"

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"
#include "storage/TableBuilder.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<SubString>("SubString");
}

void SubString::executePlanOperation() {
  auto in = std::const_pointer_cast<storage::AbstractTable>(input.getTable(0));

  storage::TableBuilder::param_list list;

  for (unsigned i = 0; i < _field_definition.size(); ++i) {
    list.append().set_type("STRING").set_name(getColName(i));
  }
  auto resultTable = storage::TableBuilder::build(list);

  resultTable->resize(in->size());

  for (size_t row = 0; row < in->size(); row++) {
    for (size_t col = 0; col < _field_definition.size(); col++) {
      resultTable->setValue<hyrise_string_t>(
          col, row, (in->getValue<std::string>(_field_definition[col], row)).substr(getStart(col), getCount(col)));
    }
  }

  addResult(std::make_shared<storage::MutableVerticalTable>(std::vector<storage::atable_ptr_t>{in, resultTable}));
}

std::shared_ptr<PlanOperation> SubString::parse(const Json::Value& data) {
  std::shared_ptr<SubString> instance = BasicParser<SubString>::parse(data);

  if (!data.isMember("fields") || data["fields"].size() < 1) {
    throw std::runtime_error("\"fields\"has to be defined");
  }

  if (data.isMember("strstart"))
    for (uint i = 0; i < data["strstart"].size(); ++i) {
      instance->addStart(data["strstart"][i].asInt());
    }

  if (data.isMember("strcount"))
    for (uint i = 0; i < data["strcount"].size(); ++i) {
      instance->addCount(data["strcount"][i].asInt());
    }

  if (data.isMember("as"))
    for (uint i = 0; i < data["as"].size(); ++i) {
      instance->addColName(data["as"][i].asString());
    }

  int fieldSize = data["fields"].size();

  if (fieldSize != instance->startSize()) {
    throw std::runtime_error("number of definitions in 'strstart' unequal to number of selected columns");
  }
  if (fieldSize != instance->countSize()) {
    throw std::runtime_error("number of definitions in 'strcount' unequal to number of selected columns");
  }
  if (fieldSize != instance->nameSize()) {
    throw std::runtime_error("number of definitions in 'as' unequal to number of selected columns");
  }

  return instance;
}

void SubString::addStart(const int& start) { _start.push_back(start); }

void SubString::addCount(const int& count) { _count.push_back(count); }

void SubString::addColName(const std::string& colName) { _colName.push_back(colName); }

int SubString::getStart(int col) const { return _start.at(col); }

int SubString::getCount(int col) const { return _count.at(col); }

std::string SubString::getColName(int col) const { return _colName.at(col); }

int SubString::startSize() const { return _start.size(); }

int SubString::countSize() const { return _count.size(); }

int SubString::nameSize() const { return _colName.size(); }
}
}
