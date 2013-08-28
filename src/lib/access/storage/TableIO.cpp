#include "TableIO.h"

#include <helper/Settings.h>

#include <io/CSVLoader.h>
#include <io/EmptyLoader.h>
#include <io/Loader.h>
#include <io/shortcuts.h>
#include <io/TableDump.h>

#include <storage/Store.h>

#include <helper/checked_cast.h>

namespace hyrise { namespace access  {

namespace {
  auto _ = QueryParser::registerPlanOperation<DumpTable>("DumpTable");
  auto _2 = QueryParser::registerPlanOperation<LoadDumpedTable>("LoadDumpedTable");
}

void DumpTable::executePlanOperation() {

  const auto& c_tab = checked_pointer_cast<const storage::Store>(getInputTable(0));

  // First merge to avoid trouble
  const auto& tab = std::const_pointer_cast<storage::Store>(c_tab);
  tab->merge();
  storage::SimpleTableDump dump(Settings::getInstance()->getDBPath());
  dump.dump(_name, tab);

  // No Output here
}

std::shared_ptr<PlanOperation> DumpTable::parse(Json::Value& data) {
  const auto& pop = std::make_shared<DumpTable>();
  pop->_name = data["name"].asString();
  return pop;
}

void LoadDumpedTable::executePlanOperation() {
  hyrise::storage::TableDumpLoader input(Settings::getInstance()->getDBPath(), _name);
  CSVHeader header(Settings::getInstance()->getDBPath() + "/" + _name + "/header.dat", CSVHeader::params().setCSVParams(csv::HYRISE_FORMAT));

  hyrise::storage::atable_ptr_t  t = Loader::load(Loader::params().setInput(input).setHeader(header));
  addResult(checked_pointer_cast<storage::Store>(t));
}

std::shared_ptr<PlanOperation> LoadDumpedTable::parse(Json::Value& data) {
  const auto& pop = std::make_shared<LoadDumpedTable>();
  pop->_name = data["name"].asString();
  return pop;
}



}}