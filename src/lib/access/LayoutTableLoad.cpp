// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LayoutTableLoad.h"

#include "access/system/QueryParser.h"

#include "helper/stringhelpers.h"

#include "io/CSVLoader.h"
#include "io/StorageManager.h"
#include "io/StringLoader.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<LayoutTableLoad>("LayoutTableLoad");
}

LayoutTableLoad::~LayoutTableLoad() {
}

void LayoutTableLoad::executePlanOperation() {
  const auto &input = this->input.getTable(0);
  if ((input->columnCount() >= 1) && (input->size() >= _input_row)) {
    std::string tmp_table_description = input->getValue<std::string>(0, _input_row);

    std::vector<std::string> description_lines;
    splitString(description_lines, tmp_table_description, "\n");

    if (_override_group != "") {
      std::vector<std::string> group_lines;
      splitString(group_lines, description_lines[2], "|");
      for (size_t index = 0; index < group_lines.size(); ++index) {
        if (_override_group == "C")
          group_lines[index] = std::to_string(index) + "_C";
        else if (_override_group == "R")
          group_lines[index] = "0_R";
        else
          throw std::runtime_error("unknown override_group " + _override_group);
      }
      description_lines[2] = joinString(group_lines, "|");
    }

    std::string table_description = joinString(description_lines, "\n");

    io::StringHeader header(table_description);
    io::CSVInput input(_file_name, io::CSVInput::params().setCSVParams(io::csv::HYRISE_FORMAT));
    auto sm = io::StorageManager::getInstance();
    sm->loadTable(_table_name, io::Loader::params().setHeader(header).setInput(input));
    addResult(sm->getTable(_table_name));
  } else {
    throw std::runtime_error("Input of LayoutTableLoad doesn't have at least one field to begin with");
  }
}

std::shared_ptr<PlanOperation> LayoutTableLoad::parse(const Json::Value &data) {
  std::shared_ptr<LayoutTableLoad> s = std::make_shared<LayoutTableLoad>();
  s->setTableName(data["table"].asString());
  s->setFileName(data["filename"].asString());
  s->setOverrideGroup(data["override_group"].asString());
  s->setInputRow(data["input_row"].asUInt());
  s->setUnsafe(data["unsafe"].asBool());
  return s;
}

const std::string LayoutTableLoad::vname() {
  return "LayoutTableLoad";
}

void LayoutTableLoad::setTableName(const std::string &tablename) {
  _table_name = tablename;
}

void LayoutTableLoad::setFileName(const std::string &filename) {
  _file_name = filename;
}

void LayoutTableLoad::setOverrideGroup(const std::string &group) {
  _override_group = group;
}

void LayoutTableLoad::setInputRow(const size_t row) {
  _input_row = row;
}

void LayoutTableLoad::setUnsafe(const bool unsafe) {
  _unsafe = unsafe;
}

}
}
