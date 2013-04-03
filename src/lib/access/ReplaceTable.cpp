// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ReplaceTable.h"

#include "access/QueryParser.h"

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
  StorageManager::getInstance()->replaceTable(_name, std::const_pointer_cast<AbstractTable>(table));
  output.add(table);
}

std::shared_ptr<_PlanOperation> ReplaceTable::parse(Json::Value& data) {
  return std::make_shared<ReplaceTable>(data["name"].asString());
}

const std::string ReplaceTable::vname() {
  return "ReplaceTable";
}

}
}
