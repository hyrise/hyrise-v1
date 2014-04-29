// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PipelineMergeHashTables.h"
#include "log4cxx/logger.h"

#include "access/MergeHashTables.h"
#include "access/PipeliningHashBuild.h"
#include "access/system/OperationData-Impl.h"

#include "storage/HashTable.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractHashTable.h"

#include "taskscheduler/SharedScheduler.h"

#include "helper/types.h"
#include "helper/vector_helpers.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PipelineMergeHashTables>("PipelineMergeHashTables");
log4cxx::LoggerPtr _pipelineLogger(log4cxx::Logger::getLogger("pipelining.PipelineMergeHashTables"));
}

void PipelineMergeHashTables::executePlanOperation() {
  auto chunks = _pipelineInput([](OperationData & pipelineInput)->std::vector<storage::c_ahashtable_ptr_t> {
    return pipelineInput.allOf<storage::AbstractHashTable>();
  });
  auto result = mergeHashTables(chunks);
  addResult(result);
}

storage::c_ahashtable_ptr_t PipelineMergeHashTables::mergeHashTables(std::vector<storage::c_ahashtable_ptr_t> tables) {
  LOG4CXX_DEBUG(_pipelineLogger, "Merging " << tables.size() << " hashes. Hashkey: " << _hashKey);
  MergeHashTables mergeHashTables;
  mergeHashTables.setKey(_hashKey);
  for (storage::c_ahashtable_ptr_t table : tables) {
    mergeHashTables.addInput(table);
  }
  mergeHashTables.execute();
  return mergeHashTables.getResultHashTable();
}

std::shared_ptr<PlanOperation> PipelineMergeHashTables::parse(const Json::Value& data) {
  return std::make_shared<PipelineMergeHashTables>();
}

void PipelineMergeHashTables::notifyNewChunk(storage::c_aresource_ptr_t chunk) {
  LOG4CXX_DEBUG(_pipelineLogger, "Got new hash chunk.");

  if (_hashKey == "") {
    _hashKey = getHashKeyFromPredecessor();
  }

  _pipelineInput([&chunk](OperationData& pipelineInput) { pipelineInput.addResource(chunk); });
}
}
}
