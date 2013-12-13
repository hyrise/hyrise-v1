// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MergeHashTables.h"

#include "access/system/QueryParser.h"

#include "storage/HashTable.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<MergeHashTables>("MergeHashTables");
}

void MergeHashTables::executePlanOperation() {
  // get first HashTable and merge subsequent tables into HashTable
  if (_key == "groupby" || _key == "selfjoin" ) {
  	if (getInputHashTable(0)->getFieldCount() == 1)
  		addResult(std::make_shared<storage::SingleAggregateHashTable>(input.getHashTables()));
  	else
  		addResult(std::make_shared<storage::AggregateHashTable>(input.getHashTables()));
  } else if (_key == "join") {
  	if (getInputHashTable(0)->getFieldCount() == 1)
  		addResult(std::make_shared<storage::SingleJoinHashTable>(input.getHashTables()));
  	else
  		addResult(std::make_shared<storage::JoinHashTable>(input.getHashTables()));
  } else {
    throw std::runtime_error("Type in Plan operation HashBuild not supported; key: " + _key);
  }
}

std::shared_ptr<PlanOperation> MergeHashTables::parse(const Json::Value &data) {
  auto instance = std::make_shared<MergeHashTables>();
  if (data.isMember("key")) {
    instance->setKey(data["key"].asString());
  }
  return instance;
}

const std::string MergeHashTables::vname() {
  return "MergeHashTables";
}

void MergeHashTables::setKey(const std::string &key) {
  _key = key;
}

const std::string MergeHashTables::getKey() const {
  return _key;
}

}
}
