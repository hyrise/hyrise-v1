#include <access/TableLoad.h>

#include "log4cxx/logger.h"

#include <io/loaders.h>
#include <io/shortcuts.h>
#include <io/StorageManager.h>

#include <access/QueryParser.h>
bool TableLoad::is_registered = QueryParser::registerPlanOperation<TableLoad>();

namespace { log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan._PlanOperation")); }

std::shared_ptr<_PlanOperation> TableLoad::parse(Json::Value &data) {
  std::shared_ptr<TableLoad> s = std::make_shared<TableLoad>();
  s->setTableName(data["table"].asString());
  s->setFileName(data["filename"].asString());
  s->setHeaderFileName(data["header"].asString());
  s->setHeaderString(data["header_string"].asString());
  s->setUnsafe(data["unsafe"].asBool());
  s->setRaw(data["raw"].asBool());
  if (data.isMember("delimiter")) {
    s->setDelimiter(data["delimiter"].asString());
  }
  return s;
}

void TableLoad::executePlanOperation() {
  StorageManager *sm = StorageManager::getInstance();
  if (!sm->exists(_table_name)) {

    // Load Raw Table
    if (_raw) {
      Loader::params p;
      p.setHeader(CSVHeader(_file_name));
      p.setInput(hyrise::io::RawTableLoader(_file_name));
      sm->loadTable(_table_name, p);

    } else if (!_header_string.empty()) {
      // Load based on header string
      Loader::params p = Loader::shortcuts::loadWithStringHeaderParams(_file_name, _header_string);
      sm->loadTable(_table_name, p);

    } else if (_header_file_name.empty()) {
      // Load only with single file
      sm->loadTableFile(_table_name, _file_name);

    } else if ((!_table_name.empty()) && (!_file_name.empty()) && (!_header_file_name.empty())) {
      // Load with dedicated header file
      Loader::params p;
      p.setCompressed(false);
      p.setHeader(CSVHeader(_header_file_name));
      auto params = CSVInput::params().setUnsafe(_unsafe);
      if (_hasDelimiter)
        params.setCSVParams(csv::params().setDelimiter(_delimiter.at(0)));
      p.setInput(CSVInput(_file_name, params));
      sm->loadTable(_table_name, p);
    }

    // We don't load unless the necessary prerequisites are met,
    // let StorageManager error if table does not exist
  } else {
    sm->getTable(_table_name);
  }
  auto _table = sm->getTable(_table_name);
  LOG4CXX_DEBUG(logger, "Loaded Table Size" << _table->size());
  addResult(_table);
}
