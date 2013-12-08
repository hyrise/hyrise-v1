// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LayoutTable.h"

#include "io/EmptyLoader.h"
#include "io/StringLoader.h"

#include "storage/SequentialHeapMerger.h"
#include "storage/Store.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<LayoutTable>("LayoutTable");
}

LayoutTable::LayoutTable(const std::string &layout) : _layout(layout) {
}

LayoutTable::~LayoutTable() {
}

void LayoutTable::executePlanOperation() {
  auto store = std::dynamic_pointer_cast<const storage::Store>(input.getTable());
  if (store == nullptr)
    throw std::runtime_error("Input is not a store");

  auto main = store->getMainTable();
  auto dest = createEmptyLayoutedTable(_layout);

  // Add all table to the game
  std::vector<storage::c_atable_ptr_t> tables { main, store->getDeltaTable() };
  
  // Call the Merge
  storage::TableMerger merger(new storage::DefaultMergeStrategy(), new storage::SequentialHeapMerger());

  // Switch the tables
  auto ntables = merger.mergeToTable(dest, tables);
  auto result = std::make_shared<storage::Store>(ntables[0]);

  output.add(result);
}

std::shared_ptr<PlanOperation> LayoutTable::parse(const Json::Value &data) {
  return std::make_shared<LayoutTable>(data["layout"].asString());
}

const std::string LayoutTable::vname() {
  return "LayoutTable";
}

storage::atable_ptr_t LayoutTable::createEmptyLayoutedTable(const std::string &layout) const {
  // Prepare the new table by defining an empty input and load the
  // partitioning for the table from the string header
  io::EmptyInput input;
  io::StringHeader header(layout);

  io::Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  p.setReturnsMutableVerticalTable(true);
  p.setReferenceTable(this->input.getTable());
  return io::Loader::load(p);
}

}
}
