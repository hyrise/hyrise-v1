// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableLoad.h"

#include <iostream>

#include "access/system/QueryParser.h"
#include "access/aging/QueryManager.h"
#include "access/aging/TableStatistic.h"

#include "io/loaders.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "io/CSVLoader.h"

#include "log4cxx/logger.h"

//#include "access/SortScan.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<TableLoad>("TableLoad");
  log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.PlanOperation"));
}

TableLoad::TableLoad(): _hasDelimiter(false),
                        _binary(false),
                        _unsafe(false),
                        _raw(false) {
}

TableLoad::~TableLoad() {
}

void TableLoad::executePlanOperation() {
  auto sm = io::StorageManager::getInstance();
  if (!sm->exists(_table_name)) {

    // Load Raw Table
    if (_raw) {
      io::Loader::params p;
      p.setHeader(io::CSVHeader(_file_name));
      p.setInput(io::RawTableLoader(_file_name));
      sm->loadTable(_table_name, p);

    } else if (!_header_string.empty()) {
      // Load based on header string
      auto p = io::Loader::shortcuts::loadWithStringHeaderParams(_file_name, _header_string);
      sm->loadTable(_table_name, p);

    } else if (_header_file_name.empty()) {
      // Load only with single file
      sm->loadTableFile(_table_name, _file_name);

    } else if ((!_table_name.empty()) && (!_file_name.empty()) && (!_header_file_name.empty())) {
      // Load with dedicated header file
      io::Loader::params p;
      p.setCompressed(false);
      p.setHeader(io::CSVHeader(_header_file_name));
      auto params = io::CSVInput::params().setUnsafe(_unsafe);
      if (_hasDelimiter)
        params.setCSVParams(io::csv::params().setDelimiter(_delimiter.at(0)));
      p.setInput(io::CSVInput(_file_name, params));
      sm->loadTable(_table_name, p);
    }

    // We don't load unless the necessary prerequisites are met,
    // let StorageManager error if table does not exist
  } else {
    sm->getTable(_table_name);
  }

  // Correct Aging information
  const auto table = sm->getTable(_table_name);
  if (_agingTables.size() != 0 && !sm->hasAgingIndex(_table_name)) {
    std::cout << "<<<<<<create AgingIndex" << std::endl;
    auto tableStatistic = std::make_shared<TableStatistic>(table);
    for (const auto& agingTable : _agingTables) {
      io::CSVInput input(sm->makePath(agingTable.second.table));
      io::CSVHeader header(sm->makePath(agingTable.second.table));
      io::Loader::params p;
      p.setInput(input);
      p.setHeader(header);
      const auto& curTable = io::Loader::load(p);
      curTable->print();

      tableStatistic->addStatisticTable(agingTable.first, curTable, agingTable.second.overRide);
      //tableStatistic->valuesDo([](query_t q, storage::field_t f, storage::value_id_t v, bool h)
       //                        {std::cout << "Q: " << q << ";F: " << f << ";V: " << v << "hot: " << h << std::endl;});
    }
    table->print();
    auto agingIndex = std::make_shared<storage::AgingIndex>(table, tableStatistic);
    sm->setAgingIndexFor(_table_name, agingIndex);
    QueryManager::instance().registerAgingIndex(agingIndex);
  }

  auto _table = sm->getTable(_table_name);
  LOG4CXX_DEBUG(logger, "Loaded Table Size" << _table->size());
  addResult(_table);
}

std::shared_ptr<PlanOperation> TableLoad::parse(const Json::Value &data) {
  std::shared_ptr<TableLoad> s = std::make_shared<TableLoad>();
  s->setTableName(data["table"].asString());
  s->setFileName(data["filename"].asString());
  s->setHeaderFileName(data["header"].asString());
  s->setHeaderString(data["header_string"].asString());
  s->setUnsafe(data["unsafe"].asBool());
  s->setRaw(data["raw"].asBool());
  if (data.isMember("aging_info")) {
    const auto& aging_info = data["aging_info"];
    for (unsigned i = 0; i < aging_info.size(); ++i) {
      const auto& cur = aging_info[i];
      bool overRide = false;
      if (!cur.isMember("field"))
        throw std::runtime_error("you need to specify a field for which to apply a table for");
      if (!cur.isMember("table"))
        throw std::runtime_error("you need to specify where to load the table from");
      if (cur.isMember("override"))
        overRide = cur["override"].asBool();
      s->addAgingTable(cur["field"].asString(), cur["table"].asString(), overRide);
    }
  }
  if (data.isMember("delimiter")) {
    s->setDelimiter(data["delimiter"].asString());
  }
  return s;
}

const std::string TableLoad::vname() {
  return "TableLoad";
}

void TableLoad::setTableName(const std::string &tablename) {
  _table_name = tablename;
}

void TableLoad::setFileName(const std::string &filename) {
  _file_name = filename;
}

void TableLoad::setHeaderFileName(const std::string &filename) {
  _header_file_name = filename;
}

void TableLoad::setHeaderString(const std::string &header) {
  _header_string = header;
}

void TableLoad::setBinary(const bool binary) {
  _binary = binary;
}

void TableLoad::setUnsafe(const bool unsafe) {
  _unsafe = unsafe;
}

void TableLoad::setRaw(const bool raw) {
  _raw = raw;
}

void TableLoad::setDelimiter(const std::string &d) {
  _delimiter = d;
  _hasDelimiter = true;
}

void TableLoad::addAgingTable(const std::string& field, const std::string& file, bool overRide) {
  const aging_table_data_t data = {file, overRide};
  const auto& r = _agingTables.insert(std::make_pair(field, data));
  if (r.second == false)
    throw std::runtime_error("double registration of field \"" + field + "\"");
}

} } // namespace hyrise::access
