// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableLoad.h"

#include "access/system/QueryParser.h"

#include "io/loaders.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"

#include "storage/Store.h"
#include "helper/checked_cast.h"

#include "log4cxx/logger.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerSerializablePlanOperation<TableLoad>("TableLoad");
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.PlanOperation"));
}


TableLoad::TableLoad(const Parameters& parameters) : _parameters(parameters) {
  if (!_parameters.path)
    _parameters.path = std::string("");
  if (!_parameters.unsafe)
    _parameters.unsafe = false;
  if (!_parameters.raw)
    _parameters.raw = false;
}

TableLoad::~TableLoad() {}

void TableLoad::executePlanOperation() {
  auto sm = io::StorageManager::getInstance();
  if (!sm->exists(_parameters.table)) {

    // load from absolute path?

    // Load Raw Table
    if (*_parameters.raw) {
      io::Loader::params p;
      p.setHeader(io::CSVHeader(_parameters.filename));
      p.setInput(io::RawTableLoader(_parameters.filename));
      sm->loadTable(_parameters.table, p, *_parameters.path);

    } else if (_parameters.header_string) {
      // Load based on header string
      auto p = io::Loader::shortcuts::loadWithStringHeaderParams(_parameters.filename, *_parameters.header_string);
      sm->loadTable(_parameters.table, p, *_parameters.path);

    } else if (!_parameters.header) {
      // Load only with single file
      sm->loadTableFile(_parameters.table, _parameters.filename, *_parameters.path);

    } else if ((!_parameters.table.empty()) && (!_parameters.filename.empty()) && (_parameters.header)) {

      // Load with dedicated header file
      io::Loader::params p;
      p.setCompressed(false);
      p.setHeader(io::CSVHeader(*_parameters.header));
      auto params = io::CSVInput::params().setUnsafe(*_parameters.unsafe);
      if (_parameters.delimiter)
        params.setCSVParams(io::csv::params().setDelimiter((*_parameters.delimiter).at(0)));
      p.setInput(io::CSVInput(_parameters.filename, params));
      sm->loadTable(_parameters.table, p, *_parameters.path);
    }
    auto table = sm->getTable(_parameters.table);
    table->setName(_parameters.table);

    // We don't load unless the necessary prerequisites are met,
    // let StorageManager error if table does not exist
  } else {
    sm->getTable(_parameters.table);
  }
  auto _table = sm->getTable(_parameters.table);
  LOG4CXX_DEBUG(logger, "Loaded Table Size" << _table->size());
  addResult(_table);
}

const std::string TableLoad::vname() { return "TableLoad"; }

void TableLoad::setTableName(const std::string& tablename) { _parameters.table = tablename; }

void TableLoad::setFileName(const std::string& filename) { _parameters.filename = filename; }

void TableLoad::setHeaderFileName(const std::string& filename) {
  _parameters.header = std::optional<std::string>(filename);
}

void TableLoad::setHeaderString(const std::string& header) {
  _parameters.header_string = std::optional<std::string>(header);
}

void TableLoad::setUnsafe(const bool unsafe) { _parameters.unsafe = std::optional<bool>(unsafe); }

void TableLoad::setRaw(const bool raw) { _parameters.raw = std::optional<bool>(raw); }

void TableLoad::setDelimiter(const std::string& d) { _parameters.delimiter = std::optional<std::string>(d); }

void TableLoad::setPath(const std::string& path) { _parameters.path = std::optional<std::string>(path); }
}
}
