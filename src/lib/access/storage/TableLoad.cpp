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
auto _ = QueryParser::registerPlanOperation<TableLoad>("TableLoad");
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.PlanOperation"));
}

TableLoad::TableLoad() : _hasDelimiter(false), _binary(false), _unsafe(false), _raw(false) {}

TableLoad::~TableLoad() {}

void TableLoad::executePlanOperation() {
  auto sm = io::StorageManager::getInstance();
  if (!sm->exists(_table_name)) {

    // load from absolute path?
    if (!_header_string.empty()) {
      // Load based on header string
      auto p = io::Loader::shortcuts::loadWithStringHeaderParams(_file_name, _header_string);
      sm->loadTable(_table_name, p, _path);

    } else if (_header_file_name.empty()) {
      // Load only with single file
      sm->loadTableFile(_table_name, _file_name, _path);

    } else if ((!_table_name.empty()) && (!_file_name.empty()) && (!_header_file_name.empty())) {

      // Load with dedicated header file
      io::Loader::params p;
      p.setCompressed(false);
      p.setHeader(io::CSVHeader(_header_file_name));
      auto params = io::CSVInput::params().setUnsafe(_unsafe);
      if (_hasDelimiter)
        params.setCSVParams(io::csv::params().setDelimiter(_delimiter.at(0)));
      p.setInput(io::CSVInput(_file_name, params));
      sm->loadTable(_table_name, p, _path);
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

std::shared_ptr<PlanOperation> TableLoad::parse(const Json::Value& data) {
  std::shared_ptr<TableLoad> s = std::make_shared<TableLoad>();
  s->setTableName(data["table"].asString());
  s->setFileName(data["filename"].asString());
  s->setHeaderFileName(data["header"].asString());
  s->setHeaderString(data["header_string"].asString());
  s->setUnsafe(data["unsafe"].asBool());
  if (data.isMember("delimiter")) {
    s->setDelimiter(data["delimiter"].asString());
  }
  if (data.isMember("path")) {
    s->setPath(data["path"].asString());
  } else {
    s->setPath("");
  }
  return s;
}

const std::string TableLoad::vname() { return "TableLoad"; }

void TableLoad::setTableName(const std::string& tablename) { _table_name = tablename; }

void TableLoad::setFileName(const std::string& filename) { _file_name = filename; }

void TableLoad::setPath(const std::string& path) { _path = path; }

void TableLoad::setHeaderFileName(const std::string& filename) { _header_file_name = filename; }

void TableLoad::setHeaderString(const std::string& header) { _header_string = header; }

void TableLoad::setBinary(const bool binary) { _binary = binary; }

void TableLoad::setUnsafe(const bool unsafe) { _unsafe = unsafe; }

void TableLoad::setDelimiter(const std::string& d) {
  _delimiter = d;
  _hasDelimiter = true;
}
}
}
