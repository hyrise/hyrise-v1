// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MergeHashTables.h"

#include "access/QueryParser.h"

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
  		addResultHash(std::make_shared<SingleAggregateHashTable>(input.getHashTables()));
  	else
  		addResultHash(std::make_shared<AggregateHashTable>(input.getHashTables()));
  } else if (_key == "join") {
  	if (getInputHashTable(0)->getFieldCount() == 1)
  		addResultHash(std::make_shared<SingleJoinHashTable>(input.getHashTables()));
  	else
  		addResultHash(std::make_shared<JoinHashTable>(input.getHashTables()));
  } else {
    throw std::runtime_error("Type in Plan operation HashBuild not supported; key: " + _key);
  }
}

std::shared_ptr<_PlanOperation> MergeHashTables::parse(Json::Value &data) {
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
