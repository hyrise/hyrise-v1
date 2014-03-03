// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PipeliningHashBuild.h"

#include "storage/HashTable.h"
#include "storage/TableRangeView.h"
#include "storage/AbstractTable.h"

#include "taskscheduler/SharedScheduler.h"
#include "access/system/ResponseTask.h"
#include "access/system/OperationData-Impl.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PipeliningHashBuild>("PipeliningHashBuild");
log4cxx::LoggerPtr _pipelineLogger(log4cxx::Logger::getLogger("pipelining.PipeliningHashBuild"));
}

void PipeliningHashBuild::executePlanOperation() {
  // if no input is available, do nothing.
  if (input.sizeOf<storage::AbstractTable>() == 0) {
    return;
  }

  size_t row_offset = 0;
  // check if table is a TableRangeView; if yes, provide the offset to HashTable
  auto input = std::dynamic_pointer_cast<const storage::TableRangeView>(getInputTable());
  if (input)
    row_offset = input->getStart();
  if (_key == "groupby" || _key == "selfjoin") {
    if (_field_definition.size() == 1)
      emitChunk(std::make_shared<storage::SingleAggregateHashTable>(getInputTable(), _field_definition, row_offset));
    else
      emitChunk(std::make_shared<storage::AggregateHashTable>(getInputTable(), _field_definition, row_offset));
  } else if (_key == "join") {
    if (_field_definition.size() == 1)
      emitChunk(std::make_shared<storage::SingleJoinHashTable>(getInputTable(), _field_definition, row_offset));
    else
      emitChunk(std::make_shared<storage::JoinHashTable>(getInputTable(), _field_definition, row_offset));
  } else {
    throw std::runtime_error("Type in Plan operation HashBuild not supported; key: " + _key);
  }
}

std::shared_ptr<PlanOperation> PipeliningHashBuild::parse(const Json::Value& data) {
  std::shared_ptr<PipeliningHashBuild> instance = std::make_shared<PipeliningHashBuild>();
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

std::shared_ptr<PlanOperation> PipeliningHashBuild::copy() {
  auto plop = std::make_shared<PipeliningHashBuild>();
  for (auto field : _indexed_field_definition) {
    plop->addField(field);
  }
  plop->setKey(getKey());
  return plop;
}

void PipeliningHashBuild::setKey(const std::string& key) { _key = key; }

const std::string PipeliningHashBuild::getKey() const { return _key; }
}
}
