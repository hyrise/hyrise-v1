#include <access/LayoutTable.h>

#include "io/shortcuts.h"
#include "io/Loader.h"
#include "io/EmptyLoader.h"
#include "io/StringLoader.h"

#include "helper/types.h"

#include "storage/SequentialHeapMerger.h"
#include "storage/SequentialHeapMergerRow.h"
#include "storage/Store.h"

bool registered = QueryParser::registerPlanOperation<LayoutTable>("LayoutTable");

std::shared_ptr<AbstractTable> LayoutTable::createEmptyLayoutedTable(const std::string &layout) const {

  // Prepare the new table by defining an empty input and load the
  // partitioning for the table from the string header
  EmptyInput input;
  StringHeader header(layout);

  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  p.setReturnsMutableVerticalTable(true);
  p.setReferenceTable(this->input.getTable());
  return Loader::load(p);

}

LayoutTable::LayoutTable(const std::string &layout) :
  _layout(layout) {}

LayoutTable::~LayoutTable() {}

void LayoutTable::executePlanOperation() {
  auto original_table = input.getTable();
  auto store = std::dynamic_pointer_cast<const Store>(original_table);

  if (store == nullptr)
    throw std::runtime_error("Input is not a store");

  auto main = store->getMainTables()[0];
  auto dest = createEmptyLayoutedTable(_layout);

  // Add all table to the game
  std::vector<hyrise::storage::c_atable_ptr_t> tables;
  tables.push_back(main);
  tables.push_back(store->getDeltaTable());

  // Call the Merge
  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  // Switch the tables
  auto ntables = merger.mergeToTable(dest, tables);
  auto result = std::make_shared<Store>(ntables[0]);

  output.add(result);
}

const std::string LayoutTable::vname() {
  return "LayoutTable";
}

std::shared_ptr<_PlanOperation> LayoutTable::parse(Json::Value &data) {
  return std::make_shared<LayoutTable>(data["layout"].asString());
}
