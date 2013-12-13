// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "ReplaceTable.h"

#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<ReplaceTable>("ReplaceTable");
}

ReplaceTable::ReplaceTable(const std::string &name) : _name(name) {
}

ReplaceTable::~ReplaceTable() {
}

void ReplaceTable::executePlanOperation() {
  auto table = input.getTable();

  // TODO: For now do the bad way, SM should have const tables
  io::StorageManager::getInstance()->replaceTable(_name, std::const_pointer_cast<storage::AbstractTable>(table));
  output.add(table);
}

std::shared_ptr<PlanOperation> ReplaceTable::parse(const Json::Value& data) {
  return std::make_shared<ReplaceTable>(data["name"].asString());
}

const std::string ReplaceTable::vname() {
  return "ReplaceTable";
}

}
}
