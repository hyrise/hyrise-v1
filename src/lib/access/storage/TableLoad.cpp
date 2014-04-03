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


TableLoad::TableLoad(const Parameters& parameters)
    : _table_name(parameters.table),
      _file_name(parameters.filename),
      _header_file_name(parameters.header),
      _header_string(parameters.header_string),
      _delimiter(parameters.delimiter),
      _path(parameters.path),
      _unsafe(parameters.unsafe),
      _raw(parameters.raw) {
  if (parameters.path)
    _path = *parameters.path;
  else
    _path = std::string("");
  if (parameters.unsafe)
    _unsafe = *parameters.unsafe;
  else
    _unsafe = false;
  if (parameters.raw)
    _raw = *parameters.raw;
  else
    _raw = false;
}

TableLoad::TableLoad() : _path(std::string("")), _unsafe(false), _raw(false), _nonvolatile(false), _binary(false) {}

TableLoad::~TableLoad() {}

void TableLoad::executePlanOperation() {
  auto sm = io::StorageManager::getInstance();
  if (!sm->exists(_table_name)) {

    // load from absolute path?

    // Load Raw Table
    if (*_raw) {
      io::Loader::params p;
      p.setHeader(io::CSVHeader(_file_name));
      p.setInput(io::RawTableLoader(_file_name));
      sm->loadTable(_table_name, p, *_path);

    } else if (_header_string) {
      // Load based on header string
      auto p = io::Loader::shortcuts::loadWithStringHeaderParams(_file_name, *_header_string);
      sm->loadTable(_table_name, p, *_path);

    } else if (!_header_file_name) {
      // Load only with single file
      sm->loadTableFile(_table_name, _file_name, *_path);

    } else if ((!_table_name.empty()) && (!_file_name.empty()) && (_header_file_name)) {

      // Load with dedicated header file
      io::Loader::params p;
      p.setCompressed(false);
      p.setHeader(io::CSVHeader(*_header_file_name));
      auto params = io::CSVInput::params().setUnsafe(*_unsafe);
      if (_delimiter)
        params.setCSVParams(io::csv::params().setDelimiter((*_delimiter).at(0)));
      p.setInput(io::CSVInput(_file_name, params));
      sm->loadTable(_table_name, p, *_path);
    }
    auto table = sm->getTable(_table_name);
    table->setName(_table_name);

    // We don't load unless the necessary prerequisites are met,
    // let StorageManager error if table does not exist
  } else {
    sm->getTable(_table_name);
  }
  auto _table = sm->getTable(_table_name);
  LOG4CXX_DEBUG(logger, "Loaded Table Size" << _table->size());
  addResult(_table);
}

const std::string TableLoad::vname() { return "TableLoad"; }

void TableLoad::setTableName(const std::string& tablename) { _table_name = tablename; }

void TableLoad::setFileName(const std::string& filename) { _file_name = filename; }

void TableLoad::setHeaderFileName(const std::string& filename) {
  _header_file_name = std::optional<std::string>(filename);
}

void TableLoad::setHeaderString(const std::string& header) { _header_string = std::optional<std::string>(header); }

void TableLoad::setUnsafe(const bool unsafe) { _unsafe = std::optional<bool>(unsafe); }

void TableLoad::setRaw(const bool raw) { _raw = std::optional<bool>(raw); }

void TableLoad::setDelimiter(const std::string& d) { _delimiter = std::optional<std::string>(d); }
void TableLoad::setPath(const std::string& path) { _path = std::optional<std::string>(path); }

void TableLoad::setBinary(const bool binary) { _binary = binary; }

void TableLoad::setNonvolatile(const bool nonvolatile) { _nonvolatile = nonvolatile; }
}
}
