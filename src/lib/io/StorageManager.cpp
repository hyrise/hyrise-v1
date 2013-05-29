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

namespace {
const char TABNS[] = "STAB";
const char MGRNS[] = "SMGR";

bool startsWith(const std::string& str, const std::string start) {
  return !str.compare(0, start.size(), start);
}

} // namespace

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
    add(SYS_STATISTICS, buildStatisticsTable());
    _initialized = true;
  }
}

StorageManager *StorageManager::getInstance() {
  static StorageManager instance;
  instance.setupSystem();
  return &instance;
}

void StorageManager::loadTable(std::string name, std::shared_ptr<AbstractTable> table) {
  add(name, table);
}

void StorageManager::replaceTable(std::string name, std::shared_ptr<AbstractTable> table) {
  replace(name, table);
}

void StorageManager::loadTable(std::string name, const Loader::params &parameters) {
  Loader::params *p = parameters.clone();
  p->setBasePath(Settings::getInstance()->getDBPath() + "/");
  addStorageTable(name, *p);
  delete p;
}

void StorageManager::loadTableFile(std::string name, std::string fileName) {
  CSVInput input(makePath(fileName));
  CSVHeader header(makePath(fileName));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  addStorageTable(name, p);
}

void StorageManager::loadTableFileWithHeader(std::string name, std::string datafileName,
                                             std::string headerFileName) {
  CSVInput input(makePath(datafileName));
  CSVHeader header(makePath(headerFileName));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  addStorageTable(name, p);
}

std::string StorageManager::makePath(std::string fileName) {
  return Settings::getInstance()->getDBPath() + "/" + fileName;
}

std::shared_ptr<AbstractTable> StorageManager::getTable(std::string name) {
  if (!exists(name)) {
    std::string tbl_file = Settings::getInstance()->getDBPath() + "/" + name + ".tbl";
    struct stat stFileInfo;

    if (stat(tbl_file.c_str(), &stFileInfo) == 0)
      loadTableFile(name, name + ".tbl");
  }
  auto table = std::dynamic_pointer_cast<AbstractTable>(get(name));
  if (table == nullptr)
    throw ResourceManagerException("StorageManager: Table '" + name + "' does not exist");
  return table;
}

void StorageManager::removeTable(std::string name) {
  if (exists(name) && std::dynamic_pointer_cast<AbstractTable>(get(name)) != nullptr)
    remove(name);
}

std::vector<std::string> StorageManager::getTableNames() const {
  std::vector<std::string> ret;
  const std::string sysPrefix("sys:");
  for (const auto &resource : _resources)
    if (std::dynamic_pointer_cast<AbstractTable>(resource.second) != nullptr &&
        !startsWith(resource.first, sysPrefix))
      ret.push_back(resource.first);
  return ret;
}

size_t StorageManager::size() const {
  size_t cnt = 0;
  const std::string sysPrefix("sys:");
  for (auto i = _resources.begin(); i != _resources.cend(); ++i)
    if (!startsWith((*i).first, sysPrefix))
     ++cnt;
  return cnt;
}

void StorageManager::removeAll() {
   const std::string sysPrefix("sys:");
   for (auto i = _resources.begin(); i != _resources.cend(); ++i)
     if (!startsWith((*i).first, sysPrefix))
       _resources.erase(i);
}

void StorageManager::printResources() const {
  std::cout << "======= Resources =======" << std::endl;
  for (const auto &kv : _resources) {
    const auto &name = kv.first;
    const auto &resource = kv.second.get();
    if (dynamic_cast<AbstractTable*>(resource) != nullptr) {
      auto table = dynamic_cast<AbstractTable*>(resource);
      std::cout << "Table "
                << table->size() << " rows "
                << table->columnCount() << " columns" << std::endl
                << "    Columns:";

      for (field_t i = 0; i != table->columnCount(); i++)
         std::cout << " " << table->metadataAt(i)->getName();
    }
    else if (dynamic_cast<AbstractIndex*>(resource) != nullptr)
      std::cout << "Index " << name;
    else
      std::cout << "Resource " << name;

    std::cout << std::endl;
  }
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
  add(name, index);
}

std::shared_ptr<AbstractIndex> StorageManager::getInvertedIndex(std::string name) {
  if (exists(name)) {
    auto index = std::dynamic_pointer_cast<AbstractIndex>(get(name));
    if (index != nullptr)
      return index;
  }
  throw ResourceManagerException("StorageManager: No index '" + name + "' found");
}

}
} // namespace hyrise::io

