// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SetTable.h"

#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SetTable>("SetTable");
}

SetTable::SetTable(const std::string& name) : _name(name) {
}

SetTable::~SetTable() {
}

void SetTable::executePlanOperation() {
  auto table = input.getTable();
  // TODO: For now do the bad way, SM should have const tables
  io::StorageManager::getInstance()->loadTable(_name, std::const_pointer_cast<storage::AbstractTable>(table));
  output.add(table);
}

std::shared_ptr<PlanOperation> SetTable::parse(const Json::Value& data) {
  return std::make_shared<SetTable>(data["name"].asString());
}

const std::string SetTable::vname() {
  return "SetTable";
}

}
}
