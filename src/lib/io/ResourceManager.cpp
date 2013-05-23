// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/ResourceManager.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <memory>

#include "helper/stringhelpers.h"
#include "helper/Environment.h"
#include "io/CSVLoader.h"
#include "storage/AbstractTable.h"
#include "storage/TableBuilder.h"

namespace hyrise {
namespace io {

//const char ResourceManager::SYS_STATISTICS[] = "sys:statistics";

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {}

std::shared_ptr<AbstractTable>  ResourceManager::buildStatisticsTable() {
  // Prepare statistics table
  TableBuilder::param_list list;
  list.append().set_type("STRING").set_name("query_id");
  list.append().set_type("STRING").set_name("operator");
  list.append().set_type("STRING").set_name("detail");
  list.append().set_type("FLOAT").set_name("selectivity");
  list.append().set_type("INTEGER").set_name("start");
  return TableBuilder::build(list);
}

void ResourceManager::setupSystem() {
  static std::mutex instance_mtx;
  std::lock_guard<std::mutex> lock(instance_mtx);
  if (!_initialized) {
    _root_path = getEnv("HYRISE_DB_PATH", "");
    // add the statistics table
    addStorageTable(SYS_STATISTICS, buildStatisticsTable());
    _initialized = true;
  }
}

ResourceManager *ResourceManager::getInstance() {
  static ResourceManager instance;
  instance.setupSystem();
  return &instance;
}

void ResourceManager::loadTable(std::string name, std::shared_ptr<AbstractTable> table) {
  addStorageTable(name, table);
}

void ResourceManager::replaceTable(std::string name, std::shared_ptr<AbstractTable> table) {
  _schema.at(name).setTable(table);
}

void ResourceManager::loadTable(std::string name, const Loader::params &parameters) {
  Loader::params *p = parameters.clone();
  p->setBasePath(_root_path + "/");
  addStorageTable(name, *p);
  delete p;
}

void ResourceManager::loadTableFile(std::string name, std::string filename) {
  CSVInput input(makePath(filename));
  CSVHeader header(makePath(filename));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  addStorageTable(name, p);
}

void ResourceManager::loadTableFileWithHeader(std::string name, std::string datafilename,
                                             std::string headerfilename) {
  CSVInput input(makePath(datafilename));
  CSVHeader header(makePath(headerfilename));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  addStorageTable(name, p);
}

std::string ResourceManager::makePath(std::string filename) {
  return _root_path + "/" + filename;
}

std::shared_ptr<AbstractTable> ResourceManager::getTable(std::string name) {
  if (!exists(name)) {
    std::string tbl_file = _root_path + "/" + name + ".tbl";
    struct stat stFileInfo;

    if (stat(tbl_file.c_str(), &stFileInfo) == 0) {
      loadTableFile(name, name + ".tbl");
    } else {
      throw StorageManagerException("ResourceManager: Table '" + name + "' does not exist");
    }
  }
  return std::dynamic_pointer_cast<AbstractTable>(_resources[name]);
}

bool ResourceManager::exists(std::string name) const {
  return _resources.count(name) >= 1;
}

void ResourceManager::assureExists(std::string name) const {
  if (!exists(name)) throw StorageManagerException("Table '" + name + "' does not exist");
}

/*void ResourceManager::preloadTable(std::string name) {
  assureExists(name);
  _schema[name].load();
}

void ResourceManager::unloadTable(std::string name) {
  assureExists(name);
  _schema[name].unload();
}*/

void ResourceManager::removeTable(std::string name) {
  if (exists(name)) {
    _schema[name].unload();
    std::lock_guard<std::mutex> lock(_schema_mutex);
    _schema.erase(name);
  }
}

std::vector<std::string> ResourceManager::getTableNames() const {
  std::vector<std::string> ret;
  for (const auto & kv : _schema)
    ret.push_back(kv.first);
  return ret;
}

namespace {
const std::string SYSTEM_PREFIX = "sys:";

bool is_not_sys_table(ResourceManager::schema_map_t::value_type i) {
  return !(i.first.compare(0, SYSTEM_PREFIX.size(), SYSTEM_PREFIX) == 0);
}
} // namespace

size_t ResourceManager::size() const {
  return std::count_if(_schema.cbegin(), _schema.cend(), is_not_sys_table);
}

/*TODO void ResourceManager::unloadAll() {
  for (auto & kv : _schema)
    kv.second.unload();
  // Clear state
  _initialized = false;
}*/

/*void ResourceManager::preloadAll() {
  for (auto & kv : _schema)
    kv.second.load();
}*/

void ResourceManager::removeAll() {
  std::lock_guard<std::mutex> lock(_schema_mutex);
  unloadAll();
  _schema.clear();
}

//TODO
/*void ResourceManager::printSchema() const {
  std::cout << "======= Schema =======" << std::endl;
  for (const auto &kv : _schema) {
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
  }
  std::cout << "====================" << std::endl;
}*/

void ResourceManager::addInvertedIndex(std::string table_name, std::shared_ptr<AbstractIndex> _index) {
  if (!exists(table_name)) {
    _indices[table_name] = _index;
  } else {
    //overwrite as of now
    _indices[table_name] = _index;
  }
}

std::shared_ptr<AbstractIndex> ResourceManager::getInvertedIndex(std::string table_name) {
  indices_t::iterator it = _indices.find(table_name);
  if (it != _indices.end()) {
    return it->second;
  } else {
    throw MissingIndexException("No such index found for table: " + table_name);
  }
}

}} // namespace hyrise::io

