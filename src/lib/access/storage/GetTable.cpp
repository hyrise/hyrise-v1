// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "GetTable.h"

#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<GetTable>("GetTable");
}

GetTable::GetTable(const std::string &name) : _name(name) {
}

GetTable::~GetTable() {
}

void GetTable::executePlanOperation() {
  output.add(io::StorageManager::getInstance()->getTable(_name));
}

std::shared_ptr<PlanOperation> GetTable::parse(const Json::Value& data) {
  return std::make_shared<GetTable>(data["name"].asString());
}

const std::string GetTable::vname() {
  return "GetTable";
}

}
}
