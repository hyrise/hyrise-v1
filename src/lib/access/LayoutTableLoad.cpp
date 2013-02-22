// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "LayoutTableLoad.h"

#include "QueryParser.h"

#include <io/StorageManager.h>
#include <io/CSVLoader.h>
#include <io/StringLoader.h>
#include <io/Loader.h>
#include <storage/AbstractTable.h>
#include <helper/stringhelpers.h>

#include <boost/lexical_cast.hpp>




bool LayoutTableLoad::is_registered = QueryParser::registerPlanOperation<LayoutTableLoad>();
LayoutTableLoad::LayoutTableLoad() {
}

LayoutTableLoad::~LayoutTableLoad() {
}

std::shared_ptr<_PlanOperation> LayoutTableLoad::parse(Json::Value &data) {
  std::shared_ptr<LayoutTableLoad> s = std::make_shared<LayoutTableLoad>();
  s->setTableName(data["table"].asString());
  s->setFileName(data["filename"].asString());
  s->setOverrideGroup(data["override_group"].asString());
  s->setInputRow(data["input_row"].asUInt());
  s->setUnsafe(data["unsafe"].asBool());
  return s;
}

void LayoutTableLoad::executePlanOperation() {
  const auto& input = this->input.getTable(0);
  if ((input->columnCount() >= 1) &&
      (input->size() >= _input_row)) {

    std::string tmp_table_description = input->getValue<std::string>(0, _input_row);
    std::vector<std::string> description_lines;
    splitString(description_lines, tmp_table_description, "\n");
    if (_override_group != "") {
      std::vector<std::string> group_lines;
      splitString(group_lines, description_lines[2], "|");
      for (size_t index = 0; index < group_lines.size(); ++index) {
        if (_override_group == "C")
          group_lines[index] = boost::lexical_cast<std::string>(index) + "_C";
        else if (_override_group == "R")
          group_lines[index] = "0_R";
        else
          throw std::runtime_error("unknown override_group " + _override_group);
      }
      description_lines[2] = joinString(group_lines, "|");
    }

    std::string table_description = joinString(description_lines, "\n");

    StringHeader header(table_description);
    CSVInput input(_file_name, CSVInput::params().setCSVParams(csv::HYRISE_FORMAT));
    StorageManager *sm = StorageManager::getInstance();
    sm->loadTable(_table_name, Loader::params().setHeader(header).setInput(input));
    addResult(sm->getTable(_table_name));
  } else {
    throw std::runtime_error("Input of LayoutTableLoad doesn't have at least one field to begin with");
  }
}



