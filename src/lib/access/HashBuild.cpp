#include "access/HashBuild.h"
#include "storage/HashTable.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<HashBuild>("HashBuild");
}

HashBuild::HashBuild() : _PlanOperation() {
}

HashBuild::~HashBuild() {
}

void HashBuild::executePlanOperation() {
  if (_key == "groupby" || _key == "selfjoin" )
    addResultHash(std::make_shared<AggregateHashTable>(getInputTable(), _field_definition));
  else if (_key == "join")
    addResultHash(std::make_shared<JoinHashTable>(getInputTable(), _field_definition));
  else {
    throw std::runtime_error("Type in Plan operation HashBuild not supported; key: " + _key);
  }
}

std::shared_ptr<_PlanOperation> HashBuild::parse(Json::Value &data) {
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

void HashBuild::setKey(const std::string &key) {
  _key = key;
}

const std::string HashBuild::getKey() const {
  return _key;
}

const std::string HashBuild::vname() {
  return "HashBuild";
}

}
}
