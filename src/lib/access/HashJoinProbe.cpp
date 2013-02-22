// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/HashJoinProbe.h"
#include "access/QueryParser.h"
#include "storage/HashTable.h"
#include "storage/PointerCalculator.h"
#include "storage/PointerCalculatorFactory.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<HashJoinProbe>("HashJoinProbe");
}

HashJoinProbe::HashJoinProbe() : _PlanOperation(), _selfjoin(false) {
}

namespace { log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan._PlanOperation")); }

void HashJoinProbe::setupPlanOperation() {
  computeDeferredIndexes();
  setBuildTable(input.getHashTable()->getTable());
}

void HashJoinProbe::executePlanOperation() {
  pos_list_t *buildTablePosList = new pos_list_t;
  pos_list_t *probeTablePosList = new pos_list_t;

  if (_selfjoin) {
    if (_field_definition.size() == 1)
      fetchPositions<SingleAggregateHashTable>(buildTablePosList, probeTablePosList);
    else  
      fetchPositions<AggregateHashTable>(buildTablePosList, probeTablePosList);
  } else {
    if (_field_definition.size() == 1) 
      fetchPositions<SingleJoinHashTable>(buildTablePosList, probeTablePosList);
    else
      fetchPositions<JoinHashTable>(buildTablePosList, probeTablePosList);
  }

  addResult(buildResultTable(buildTablePosList, probeTablePosList));
}

const std::string HashJoinProbe::vname() {
  return "HashJoinProbe";
}

template<class HashTable>
void HashJoinProbe::fetchPositions(pos_list_t *buildTablePosList,
                                   pos_list_t *probeTablePosList) {

  const auto& probeTable = getProbeTable();
  const auto& hash_table = std::dynamic_pointer_cast<const HashTable>(getInputHashTable(0));
  assert(hash_table != nullptr);

  LOG4CXX_DEBUG(logger, hash_table->stats());
  LOG4CXX_DEBUG(logger, "Probe Table Size: " << probeTable->size());
  LOG4CXX_DEBUG(logger, "Hash Table Size:  " << hash_table->size());

  //probeTable->print(1);

  for (pos_t probeTableRow = 0; probeTableRow < probeTable->size(); ++probeTableRow) {
    pos_list_t matchingRows(hash_table->get(probeTable, _field_definition, probeTableRow));

    if (!matchingRows.empty()) {
      buildTablePosList->insert(buildTablePosList->end(), matchingRows.begin(), matchingRows.end());
      probeTablePosList->insert(probeTablePosList->end(), matchingRows.size(), probeTableRow);
    }
  }

  LOG4CXX_DEBUG(logger, "Done Probing");
}

hyrise::storage::atable_ptr_t HashJoinProbe::buildResultTable(
  pos_list_t *buildTablePosList,
  pos_list_t *probeTablePosList) const {
  std::vector<hyrise::storage::atable_ptr_t > parts;

  auto
  buildTableRows = PointerCalculatorFactory::createPointerCalculatorNonRef(getBuildTable(), nullptr, buildTablePosList),
  probeTableRows = PointerCalculatorFactory::createPointerCalculatorNonRef(getProbeTable(), nullptr, probeTablePosList);

  parts.push_back(probeTableRows);
  parts.push_back(buildTableRows);

  hyrise::storage::atable_ptr_t result = std::make_shared<MutableVerticalTable>(parts);
  return result;
}

void HashJoinProbe::setBuildTable(hyrise::storage::c_atable_ptr_t table) {
  _buildTable = table;
}

hyrise::storage::c_atable_ptr_t HashJoinProbe::getBuildTable() const {
  return _buildTable;
}

hyrise::storage::c_atable_ptr_t HashJoinProbe::getProbeTable() const {
  return getInputTable();
}

std::shared_ptr<_PlanOperation> HashJoinProbe::parse(Json::Value &data) {
  std::shared_ptr<HashJoinProbe> instance = std::make_shared<HashJoinProbe>();
  if (data.isMember("fields")) {
    for (unsigned i = 0; i < data["fields"].size(); ++i) {
      instance->addField(data["fields"][i]);
    }
  }
  instance->_selfjoin = data["selfjoin"].asBool();
  return instance;
}

}
}
