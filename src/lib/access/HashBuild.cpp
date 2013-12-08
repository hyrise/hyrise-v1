// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/HashBuild.h"

#include "storage/HashTable.h"
#include "storage/TableRangeView.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<HashBuild>("HashBuild");
}

HashBuild::~HashBuild() {
}

void HashBuild::executePlanOperation() {
  size_t row_offset = 0;
  // check if table is a TableRangeView; if yes, provide the offset to HashTable
  auto input = std::dynamic_pointer_cast<const storage::TableRangeView>(getInputTable());
  if(input)
    row_offset = input->getStart();
  if (_key == "groupby" || _key == "selfjoin" ) {
    if (_field_definition.size() == 1)
        addResult(std::make_shared<storage::SingleAggregateHashTable>(getInputTable(), _field_definition, row_offset));
      else
        addResult(std::make_shared<storage::AggregateHashTable>(getInputTable(), _field_definition, row_offset));
  } else if (_key == "join") {
    if (_field_definition.size() == 1)
      addResult(std::make_shared<storage::SingleJoinHashTable>(getInputTable(), _field_definition, row_offset));
    else
      addResult(std::make_shared<storage::JoinHashTable>(getInputTable(), _field_definition, row_offset));
  } else {
    throw std::runtime_error("Type in Plan operation HashBuild not supported; key: " + _key);
  }
}

std::shared_ptr<PlanOperation> HashBuild::parse(const Json::Value &data) {
  std::shared_ptr<HashBuild> instance = std::make_shared<HashBuild>();
  if (data.isMember("fields")) {
    for (unsigned i = 0; i < data["fields"].size(); ++i) {
      instance->addField(data["fields"][i]);
    }
  }
  if (data.isMember("key")) {
    instance->setKey(data["key"].asString());
  }
  return instance;
}

const std::string HashBuild::vname() {
  return "HashBuild";
}

void HashBuild::setKey(const std::string &key) {
  _key = key;
}

const std::string HashBuild::getKey() const {
  return _key;
}

}
}
