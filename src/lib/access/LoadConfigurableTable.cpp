// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadConfigurableTable.h"

#include <stdexcept>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

#include "access/QueryParser.h"

#include "io/shortcuts.h"
#include "io/StorageManager.h"

#include <iostream>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<LoadConfigurableTable>("LoadConfigurableTable");
}

LoadConfigurableTable::LoadConfigurableTable(const std::string &filename, const PColumnProperties colProperties) : 
  _filename(filename),
  _colProperties(colProperties){ 
  }

LoadConfigurableTable::~LoadConfigurableTable() {
}

void LoadConfigurableTable::executePlanOperation() {
  Loader::params p;
  p.setColProperties(_colProperties);
  output.add(Loader::shortcuts::load(
        StorageManager::getInstance()->makePath(_filename),
        p));
}

std::shared_ptr<_PlanOperation> LoadConfigurableTable::parse(Json::Value &data) {
  if (data["filename"].asString().empty())
    throw std::runtime_error("LoadConfigurableTable invalid without \"filename\": ...");

  PColumnProperties colProps = new ColumnProperties;
  if ( !data["defaultColumnType"].asString().empty() )
    colProps->defaultType = ColumnProperties::typeFromString(data["defaultColumnType"].asString());

  if ( !data["columnTypes"].asString().empty() ) {
    std::vector<std::string> typeFields;
    std::string columnTypes = data["columnTypes"].asString();
    boost::split( typeFields, columnTypes, boost::is_any_of(".") );
    auto it = typeFields.begin();
    auto itEnd = typeFields.end();
    for (; it != itEnd; ++it) {
      std::vector<std::string> columnList;
      boost::split( columnList, *it, boost::is_any_of(","));
      ColumnType currentType = ColumnProperties::typeFromString(columnList[0]);
      auto itType = columnList.begin() + 1;
      auto itTypeEnd = columnList.end();
      for (; itType != itTypeEnd; ++itType) {
        colProps->setType(atoi(itType->c_str()), currentType);
      }
    }
  }

  return std::make_shared<LoadConfigurableTable>(
      data["filename"].asString(),
      colProps);
}

const std::string LoadConfigurableTable::vname() {
  return "LoadConfigurableTable";
}

}
}
