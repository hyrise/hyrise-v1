// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/StorageManager.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "helper/Settings.h"
#include "helper/stringhelpers.h"
#include "helper/Environment.h"
#include "io/CSVLoader.h"
#include "storage/AbstractIndex.h"
#include "storage/AbstractTable.h"
#include "storage/TableBuilder.h"

namespace hyrise {
namespace io {

const char StorageManager::SYS_STATISTICS[] = "sys:statistics";

const char TABNS[] = "STAB";
const char MGRNS[] = "SMGR";

/*StorageTable::StorageTable() {}

StorageTable::StorageTable(std::string table_name)
    : _name(table_name),
      _parameters(nullptr) {}

StorageTable::StorageTable(std::string table_name, std::shared_ptr<AbstractTable> table)
    : _name(table_name),
      _table(table),
      _parameters(nullptr) {}

StorageTable::StorageTable(std::string table_name, const Loader::params &parameters)
    : _name(table_name),
      _table(nullptr),
      _parameters(parameters.clone()) {}

StorageTable::StorageTable(const StorageTable &st)
    : _name(st._name),
      _table(st._table),
      _parameters(st._parameters ? st._parameters->clone() : nullptr) {}

StorageTable::~StorageTable() {}

void StorageTable::load() {
  std::lock_guard<std::mutex> lock(_table_mutex);
  if (isLoaded()) return;

  if (isLoadable()) {
    _table = Loader::load(*_parameters);
  } else {
    throw ResourceManagerException("Could not load table '" + _name + "'");
  }
}

void StorageTable::unload() {
  std::lock_guard<std::mutex> lock(_table_mutex);
  _table.reset();
}

std::shared_ptr<AbstractTable> StorageTable::getTable() {
  if (!isLoaded())
    load();
  return _table;
}

std::shared_ptr<AbstractTable> StorageTable::getTable() const {
  return _table;
}

void StorageTable::setTable(std::shared_ptr<AbstractTable> table) {
  std::lock_guard<std::mutex> lock(_table_mutex);
  _table = table;
}

bool StorageTable::isLoaded() const {
  return _table != nullptr;
}

bool StorageTable::isLoadable() const {
  return (_parameters != nullptr);
}*/




StorageManager::StorageManager() {}

StorageManager::~StorageManager() {}

std::shared_ptr<AbstractTable>  StorageManager::buildStatisticsTable() {
  // Prepare statistics table
  TableBuilder::param_list list;
  list.append().set_type("STRING").set_name("query_id");
  list.append().set_type("STRING").set_name("operator");
  list.append().set_type("STRING").set_name("detail");
  list.append().set_type("FLOAT").set_name("selectivity");
  list.append().set_type("INTEGER").set_name("start");
  return TableBuilder::build(list);
}

void StorageManager::setupSystem() {
  static std::mutex instance_mtx;
  std::lock_guard<std::mutex> lock(instance_mtx);
  if (!_initialized) {
    // add the statistics table
    add<AbstractTable>(SYS_STATISTICS, buildStatisticsTable());
    _initialized = true;
  }
}

StorageManager *StorageManager::getInstance() {
  static StorageManager instance;
  instance.setupSystem();
  return &instance;
}

void StorageManager::loadTable(std::string name, std::shared_ptr<AbstractTable> table) {
  add<AbstractTable>(name, table);
}

void StorageManager::replaceTable(std::string name, std::shared_ptr<AbstractTable> table) {
  //TODO
  //_schema.at(name).setTable(table);
}

void StorageManager::loadTable(std::string name, const Loader::params &parameters) {
  Loader::params *p = parameters.clone();
  p->setBasePath(Settings::getInstance()->getDBPath() + "/");
  addStorageTable(name, *p);
  delete p;
}

void StorageManager::loadTableFile(std::string name, std::string filename) {
  CSVInput input(makePath(filename));
  CSVHeader header(makePath(filename));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  addStorageTable(name, p);
}

void StorageManager::loadTableFileWithHeader(std::string name, std::string datafilename,
                                             std::string headerfilename) {
  CSVInput input(makePath(datafilename));
  CSVHeader header(makePath(headerfilename));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  addStorageTable(name, p);
}

std::string StorageManager::makePath(std::string filename) {
  return Settings::getInstance()->getDBPath() + "/" + filename;
}

std::shared_ptr<AbstractTable> StorageManager::getTable(std::string name) {
  if (!exists(name)) {
    std::string tbl_file = Settings::getInstance()->getDBPath() + "/" + name + ".tbl";
    struct stat stFileInfo;

    if (stat(tbl_file.c_str(), &stFileInfo) == 0) {
      loadTableFile(name, name + ".tbl");
    } else {
      throw ResourceManagerException("StorageManager: Table '" + name + "' does not exist");
    }
  }
  return get<AbstractTable>(name);
}

void StorageManager::removeTable(std::string name) {
  if (exists<AbstractTable>(name))
    remove<AbstractTable>(name);
}

std::vector<std::string> StorageManager::getTableNames() const {
  //TODO
  std::vector<std::string> ret;
  /*for (const auto &kv : _schema)
    ret.push_back(kv.first);*/
  return ret;
}

namespace {
const std::string SYSTEM_PREFIX = "sys:";

/*bool is_not_sys_table(StorageManager::schema_map_t::value_type i) {
  return i.first.compare(0, SYSTEM_PREFIX.size(), SYSTEM_PREFIX) == 0;
}*/
} // namespace

size_t StorageManager::size() const {
  //TODO
  return 0;//std::count_if(_schema.cbegin(), _schema.cend(), is_not_sys_table);
}

void StorageManager::removeAll() {
  clear();
}

void StorageManager::printSchema() const {
  std::cout << "======= Schema =======" << std::endl;
  /*for (const auto &kv : _schema) {
    const auto &name = kv.first;
    const auto &storage_table = kv.second;
    std::cout << "Table " << name << " ";

    if (storage_table.isLoaded()) {
      auto t = storage_table.getTable();
      std::cout << "[Loaded] "
                << t->size() << " rows "
                << t->columnCount() << " columns"
                << std::endl
                << "    Columns:";

      for (field_t i = 0; i != t->columnCount(); i++) {
        std::cout << " " << t->metadataAt(i)->getName();
      }

    } else {
      if (storage_table.isLoadable()) {
        std::cout << " [Loadable]";
      } else {
        std::cout << " [Dead]";
      }
    }
    std::cout << std::endl;
  }*/
  std::cout << "====================" << std::endl;
}

std::vector<std::string> StorageManager::listDirectory(std::string dir) {
  std::vector<std::string> files;
  DIR *dp;
  struct dirent *dirp;

  if ((dp  = opendir(dir.c_str())) == nullptr)
    throw ResourceManagerException("Error opening directory: '" + dir + "'");

  while ((dirp = readdir(dp)) != nullptr)
    files.push_back(std::string(dirp->d_name));

  closedir(dp);
  return files;
}

void StorageManager::addInvertedIndex(std::string name, std::shared_ptr<AbstractIndex> index) {
  add<AbstractIndex>(name, index);
}

std::shared_ptr<AbstractIndex> StorageManager::getInvertedIndex(std::string name) {
  if (exists<AbstractIndex>(name))
    return get<AbstractIndex>(name);
  else
    throw ResourceManagerException("StorageManager: No index found for table '" + name + "'");
}

}} // namespace hyrise::io

