// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "GroupByScan.h"
#include "QueryParser.h"
#include <storage/HashTable.h>
#include <storage/PrettyPrinter.h>
#include "default_strategy.h"

#include <algorithm>

namespace hyrise {
namespace access {

bool GroupByScan::is_registered = QueryParser::registerPlanOperation<GroupByScan>();

std::shared_ptr<_PlanOperation> GroupByScan::parse(Json::Value &v) {
  std::shared_ptr<GroupByScan> gs = std::make_shared<GroupByScan>();

  if (v.isMember("fields")) {
    for (unsigned i = 0; i <  v["fields"].size(); ++i) {
      gs->addField(v["fields"][i]);
    }
  }

  if (v.isMember("functions")) {
    for (unsigned i = 0; i < v["functions"].size(); ++i) {
      const Json::Value &f = v["functions"][i];
      gs->addFunction(parseAggregateFunction(f));
    }
  }
  return gs;
}

GroupByScan::GroupByScan(): _PlanOperation() {}

GroupByScan::~GroupByScan() {
for (auto e : aggregate_functions)
    delete e;
}

hyrise::storage::atable_ptr_t GroupByScan::createResultTableLayout() {
  metadata_list  metadata;
  std::vector<AbstractTable::SharedDictionaryPtr> dictionaries;
  //creating fields from grouping fields
  hyrise::storage::atable_ptr_t group_tab = getInputTable(0)->copy_structure_modifiable(&_field_definition);
  //creating fields from aggregate functions
  for (const auto & fun: aggregate_functions) {
    ColumnMetadata *m = new ColumnMetadata(fun->columnName(getInputTable(0)->nameOfColumn(fun->getField())), fun->getType());
    metadata.push_back(m);
    dictionaries.push_back(AbstractDictionary::dictionaryWithType<DictionaryFactory<OrderIndifferentDictionary> >(fun->getType()));
  }
  hyrise::storage::atable_ptr_t agg_tab = std::make_shared<Table<DEFAULT_STRATEGY>>(&metadata, &dictionaries, 0, false);

  //Clean the metadata
  for (auto e : metadata)
    delete e;

  std::vector<hyrise::storage::atable_ptr_t > vc;
  if (_field_definition.size() == 0 && aggregate_functions.size() != 0) {
    return agg_tab;
  } else if (_field_definition.size() != 0 && aggregate_functions.size() == 0) {
    return group_tab;
  } else {
    vc.push_back(group_tab);
    vc.push_back(agg_tab);
    hyrise::storage::atable_ptr_t result = std::make_shared<MutableVerticalTable>(vc);
    return result;
  }
}

void GroupByScan::splitInput() {
  hash_table_list_t& hashTables = input.getHashTables();  
  if (_count > 0 && !hashTables.empty()) {
    u_int64_t first, last;
    distribute(hashTables[0]->numKeys(), first, last);

    if ((_indexed_field_definition.size() + _named_field_definition.size()) == 1)
      hashTables[0] = std::dynamic_pointer_cast<SingleAggregateHashTable>(hashTables[0])->view(first, last + 1);
    else
      hashTables[0] = std::dynamic_pointer_cast<AggregateHashTable>(hashTables[0])->view(first, last + 1);
  }
}

void GroupByScan::setupPlanOperation() {
  _PlanOperation::setupPlanOperation();

  const auto& t = getInputTable(0);
  for (const auto & function: aggregate_functions) {
    function->walk(*t);
  }
}

void GroupByScan::writeGroupResult(hyrise::storage::atable_ptr_t resultTab, std::shared_ptr<pos_list_t> hit, size_t row) {
  for (const auto & columnNr: _field_definition) {
    hyrise::storage::write_group_functor fun(getInputTable(0), resultTab, hit->at(0), (size_t)columnNr, row);
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    ts(getInputTable(0)->typeOfColumn(columnNr), fun);
  }

for (const auto & funct: aggregate_functions) {
    funct->processValuesForRows(getInputTable(0), hit.get(), resultTab, row);
  }
}

template<typename HashTableType, typename MapType, typename KeyType>
void GroupByScan::executeGroupBy() {
  auto resultTab = createResultTableLayout();
  auto groupResults = getInputHashTable();
  // Allocate some memory for the result tab and resize the table    
  resultTab->resize(groupResults->numKeys());

  pos_t row = 0;
  typename HashTableType::map_const_iterator_t it1, it2, end;
  // set iterators: in the sequential case, getInputTable() returns an AggregateHashTable, in the parallel case a HashTableView<>
  // Alternatively, a common type could be introduced
  if (_count < 1) {
    auto aggregateHashTable = std::dynamic_pointer_cast<HashTableType>(groupResults);
    it1 = aggregateHashTable->getMapBegin();
    end = aggregateHashTable->getMapEnd();
  } else {
    auto hashTableView = std::dynamic_pointer_cast<HashTableView<MapType, KeyType> >(groupResults);
    it1 = hashTableView->getMapBegin();
    end = hashTableView->getMapEnd();
  }
  for (it2 = it1; it1 != end; it1 = it2) {
    // outer loop over unique keys
    auto pos_list = std::make_shared<pos_list_t>();
    for (; (it2 != end) && (it1->first == it2->first); ++it2) {
      // inner loop, all keys equal to it1->first
      pos_list->push_back(it2->second);
    }
    writeGroupResult(resultTab, pos_list, row);
    row++;
  }
  this->addResult(resultTab);
}

void GroupByScan::executePlanOperation() {
  

  if ((_field_definition.size() != 0) && (input.numberOfHashTables() >= 1)) {
    if (_field_definition.size() == 1)
      return executeGroupBy<SingleAggregateHashTable, aggregate_single_hash_map_t, aggregate_single_key_t>();
    else
      return executeGroupBy<AggregateHashTable, aggregate_hash_map_t, aggregate_key_t>();
  } else {
    auto resultTab = createResultTableLayout();
    resultTab->resize(1);
    for (const auto & funct: aggregate_functions) {
      funct->processValuesForRows(getInputTable(0), nullptr, resultTab, 0);
    }
    this->addResult(resultTab);
  }
}

}}
