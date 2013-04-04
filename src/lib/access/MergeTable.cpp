// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MergeTable.h"

#include "access/QueryParser.h"

#include "storage/Store.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<MergeTable>("MergeTable");
}

MergeTable::~MergeTable() {
}

void MergeTable::executePlanOperation() {
  std::vector<storage::c_atable_ptr_t> tables;
  // Add all tables to the game
  for (auto& table: input.getTables()) {
    if (auto store = std::dynamic_pointer_cast<const Store>(table)) {
      auto mains = store->getMainTables();
      tables.insert(tables.end(), mains.begin(), mains.end());
      tables.push_back(store->getDeltaTable());
    } else {
      tables.push_back(table);
    }
  }

  // Call the Merge
  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  auto new_table = input.getTable(0)->copy_structure();

  // Switch the tables
  auto merged_tables = merger.mergeToTable(new_table, tables);
  const auto &result = std::make_shared<Store>(new_table);

  output.add(result);
}

std::shared_ptr<_PlanOperation> MergeTable::parse(Json::Value& data) {
  return std::make_shared<MergeTable>();
}

const std::string MergeTable::vname() {
  return "MergeTable";
}

}
}
