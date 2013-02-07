#include "access/GetTable.h"
#include "access/QueryParser.h"
#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<GetTable>("GetTable");
}

GetTable::GetTable(const std::string& name) : _name(name) {
}

GetTable::~GetTable() {
}

std::shared_ptr<_PlanOperation> GetTable::parse(Json::Value& data) {
  return std::make_shared<GetTable>(data["name"].asString());
}

const std::string GetTable::vname() {
  return "GetTable";
}

void GetTable::executePlanOperation() {
  output.add(StorageManager::getInstance()->getTable(_name));
}

}
}
