// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/HashJoinProbe.h"

#include "access/system/QueryParser.h"

#include "storage/HashTable.h"
#include "storage/PointerCalculator.h"

#include <log4cxx/logger.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<HashJoinProbe>("HashJoinProbe");
  log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.PlanOperation"));
}

HashJoinProbe::HashJoinProbe() : _selfjoin(false) {
}

HashJoinProbe::~HashJoinProbe() {
}

void HashJoinProbe::setupPlanOperation() {
  PlanOperation::setupPlanOperation();
  setBuildTable(getInputHashTable()->getTable());
}

void HashJoinProbe::executePlanOperation() {
  storage::pos_list_t *buildTablePosList = new pos_list_t;
  storage::pos_list_t *probeTablePosList = new pos_list_t;

  if (_selfjoin) {
    if (_field_definition.size() == 1)
      fetchPositions<storage::SingleAggregateHashTable>(buildTablePosList, probeTablePosList);
    else
      fetchPositions<storage::AggregateHashTable>(buildTablePosList, probeTablePosList);
  } else {
    if (_field_definition.size() == 1)
      fetchPositions<storage::SingleJoinHashTable>(buildTablePosList, probeTablePosList);
    else
      fetchPositions<storage::JoinHashTable>(buildTablePosList, probeTablePosList);
  }

  addResult(buildResultTable(buildTablePosList, probeTablePosList));
}

std::shared_ptr<PlanOperation> HashJoinProbe::parse(const Json::Value &data) {
  std::shared_ptr<HashJoinProbe> instance = std::make_shared<HashJoinProbe>();
  if (data.isMember("fields")) {
    for (unsigned i = 0; i < data["fields"].size(); ++i) {
      instance->addField(data["fields"][i]);
    }
  }
  instance->_selfjoin = data["selfjoin"].asBool();
  return instance;
}

const std::string HashJoinProbe::vname() {
  return "HashJoinProbe";
}

void HashJoinProbe::setBuildTable(const storage::c_atable_ptr_t &table) {
  _buildTable = table;
}

storage::c_atable_ptr_t HashJoinProbe::getBuildTable() const {
  return _buildTable;
}

storage::c_atable_ptr_t HashJoinProbe::getProbeTable() const {
  return getInputTable();
}

template<class HashTable>
void HashJoinProbe::fetchPositions(storage::pos_list_t *buildTablePosList,
                                   storage::pos_list_t *probeTablePosList) {
  const auto& probeTable = getProbeTable();
  const auto& hash_table = std::dynamic_pointer_cast<const HashTable>(getInputHashTable(0));
  assert(hash_table != nullptr);

  LOG4CXX_DEBUG(logger, hash_table->stats());
  LOG4CXX_DEBUG(logger, "Probe Table Size: " << probeTable->size());
  LOG4CXX_DEBUG(logger, "Hash Table Size:  " << hash_table->size());

  for (pos_t probeTableRow = 0; probeTableRow < probeTable->size(); ++probeTableRow) {
    pos_list_t matchingRows(hash_table->get(probeTable, _field_definition, probeTableRow));

    if (!matchingRows.empty()) {
      buildTablePosList->insert(buildTablePosList->end(), matchingRows.begin(), matchingRows.end());
      probeTablePosList->insert(probeTablePosList->end(), matchingRows.size(), probeTableRow);
    }
  }

  LOG4CXX_DEBUG(logger, "Done Probing");
}

storage::atable_ptr_t HashJoinProbe::buildResultTable(storage::pos_list_t *buildTablePosList,
                                                      storage::pos_list_t *probeTablePosList) const {
  std::vector<storage::atable_ptr_t> parts;

  auto buildTableRows = storage::PointerCalculator::create(getBuildTable(), buildTablePosList);
  auto probeTableRows = storage::PointerCalculator::create(getProbeTable(), probeTablePosList);

  parts.push_back(probeTableRows);
  parts.push_back(buildTableRows);

  storage::atable_ptr_t result = std::make_shared<storage::MutableVerticalTable>(parts);
  return result;
}

}
}
