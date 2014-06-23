// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PipeliningHashProbe.h"

#include "access/system/QueryParser.h"
#include "access/PipelineObserver.h"
#include "access/system/OperationData.h"
#include "access/system/OperationData-Impl.h"

#include "helper/types.h"

#include "storage/HashTable.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"

#include <log4cxx/logger.h>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PipeliningHashProbe>("PipeliningHashProbe");
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.PlanOperation"));
log4cxx::LoggerPtr pipelineLogger(log4cxx::Logger::getLogger("pipelining.PipeliningHashProbe"));
}

PipeliningHashProbe::PipeliningHashProbe() : _selfjoin(false) {}

void PipeliningHashProbe::setupPlanOperation() {
  PlanOperation::setupPlanOperation();
  setBuildTable(getInputHashTable()->getTable());
}

void PipeliningHashProbe::executePlanOperation() {
  LOG4CXX_DEBUG(pipelineLogger, _operatorId << ": executePlanOperation");

  // if there is no input
  // this is just a stub operation and all work
  // has been done in chunk workers.
  if (input.sizeOf<storage::AbstractTable>() == 0) {
    LOG4CXX_DEBUG(pipelineLogger, _operatorId << ": No input found. Continuing.");
    return;
  }

  resetPosLists();

  if (_selfjoin) {
    if (_field_definition.size() == 1)
      fetchPositions<storage::SingleAggregateHashTable>();
    else
      fetchPositions<storage::AggregateHashTable>();
  } else {
    if (_field_definition.size() == 1)
      fetchPositions<storage::SingleJoinHashTable>();
    else
      fetchPositions<storage::JoinHashTable>();
  }
  LOG4CXX_DEBUG(pipelineLogger, _operatorId << ": executePlanOperation done");
}

std::shared_ptr<PlanOperation> PipeliningHashProbe::parse(const Json::Value& data) {
  std::shared_ptr<PipeliningHashProbe> instance = std::make_shared<PipeliningHashProbe>();
  if (data.isMember("fields")) {
    for (unsigned i = 0; i < data["fields"].size(); ++i) {
      instance->addField(data["fields"][i]);
    }
  }
  instance->_selfjoin = data["selfjoin"].asBool();
  instance->_chunkSize = data["chunkSize"].asUInt();
  return instance;
}

void PipeliningHashProbe::setBuildTable(const storage::c_atable_ptr_t& table) { _buildTable = table; }

storage::c_atable_ptr_t PipeliningHashProbe::getBuildTable() const { return _buildTable; }

storage::c_atable_ptr_t PipeliningHashProbe::getProbeTable() const { return getInputTable(); }

std::shared_ptr<PlanOperation> PipeliningHashProbe::copy() {
  std::shared_ptr<PipeliningHashProbe> instance = std::make_shared<PipeliningHashProbe>();
  for (auto field : _indexed_field_definition) {
    instance->addField(field);
  }
  instance->_selfjoin = _selfjoin;
  instance->_chunkSize = _chunkSize;
  return instance;
}

void PipeliningHashProbe::addCustomDependencies(taskscheduler::task_ptr_t newChunkTask) {
  // this new chunk task needs HashBuild as dependency.
  newChunkTask->addDependency(getHashBuildPredecessor());
}

template <class HashTable>
void PipeliningHashProbe::fetchPositions() {
  const auto& probeTable = getProbeTable();
  const auto& hash_table = std::dynamic_pointer_cast<const HashTable>(getInputHashTable(0));
  assert(hash_table != nullptr);

  LOG4CXX_DEBUG(logger, hash_table->stats());
  LOG4CXX_DEBUG(logger, "Probe Table Size: " << probeTable->size());
  LOG4CXX_DEBUG(logger, "Hash Table Size:  " << hash_table->size());

  for (pos_t probeTableRow = 0; probeTableRow < probeTable->size(); ++probeTableRow) {
    pos_list_t matchingRows(hash_table->get(probeTable, _field_definition, probeTableRow));

    if (!matchingRows.empty()) {
      _buildTablePosList->insert(_buildTablePosList->end(), matchingRows.begin(), matchingRows.end());
      _probeTablePosList->insert(_probeTablePosList->end(), matchingRows.size(), probeTableRow);
    }

    // We can only monitor the size of the output after each row has been probed.
    // Each probed row can produce n matches. The output table will increase by those n rows.
    // As soon as the current n causes the output table to reach the _chunkSize threshold,
    // we will emit a chunk and reset the _buildTablePosList and _probeTablePosList.
    if (_buildTablePosList->size() > _chunkSize) {
      createAndEmitChunk();
    }
  }

  // Emit final results.
  if (_buildTablePosList->size() > 0) {
    createAndEmitChunk();
  }

  LOG4CXX_DEBUG(logger, "Done Probing");
}

void PipeliningHashProbe::createAndEmitChunk() {
  LOG4CXX_DEBUG(pipelineLogger, _operatorId << ": Emitting chunk.");
  PipelineEmitter<PipeliningHashProbe>::emitChunk(buildResultTable());
  // reset the positions lists
  resetPosLists();
}

storage::atable_ptr_t PipeliningHashProbe::buildResultTable() const {
  std::vector<storage::atable_ptr_t> parts;

  auto buildTableRows = storage::PointerCalculator::create(getBuildTable(), std::move(_buildTablePosList));
  auto probeTableRows = storage::PointerCalculator::create(getProbeTable(), std::move(_probeTablePosList));

  parts.push_back(probeTableRows);
  parts.push_back(buildTableRows);

  storage::atable_ptr_t result = std::make_shared<storage::MutableVerticalTable>(parts);
  return result;
}

void PipeliningHashProbe::resetPosLists() {
  _buildTablePosList = new pos_list_t;
  _probeTablePosList = new pos_list_t;
}
}
}
