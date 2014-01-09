// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/StorageManager.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "helper/Settings.h"
#include "helper/stringhelpers.h"
#include "helper/Environment.h"
#include "io/Loader.h"
#include "io/CSVLoader.h"
#include "storage/AbstractIndex.h"
#include "storage/AbstractTable.h"
#include "storage/ColumnMetadata.h"
#include "storage/TableBuilder.h"

namespace hyrise {
namespace io {

template<typename... Args>
void StorageManager::addStorageTable(std::string name, Args && ... args) {
  add(name, Loader::load(std::forward<Args>(args)...));
}

StorageManager *StorageManager::getInstance() {
  // TODO: This is a hack for the moment, since StorageManager only adds to the interface
  // but does not have its own members. This keeps the old interface intact until we migrate
  // remaining code.
  return static_cast<StorageManager*>(&ResourceManager::getInstance());
}

void StorageManager::loadTable(std::string name, std::shared_ptr<storage::AbstractTable> table) {
  add(name, table);
}

void StorageManager::replaceTable(std::string name, std::shared_ptr<storage::AbstractTable> table) {
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

std::shared_ptr<storage::AbstractTable> StorageManager::getTable(std::string name) {
  if (!exists(name)) {
    std::string tbl_file = Settings::getInstance()->getDBPath() + "/" + name + ".tbl";
    struct stat stFileInfo;

    if (stat(tbl_file.c_str(), &stFileInfo) == 0)
      loadTableFile(name, name + ".tbl");
  }
  return get<storage::AbstractTable>(name);
}

void StorageManager::removeTable(std::string name) {
  if (exists(name))
    remove(name);
}

std::vector<std::string> StorageManager::getTableNames() const {
  std::vector<std::string> ret;
  for (const auto &resource : all())
    if (std::dynamic_pointer_cast<storage::AbstractTable>(resource.second) != nullptr)
      ret.push_back(resource.first);
  return ret;
}

void StorageManager::removeAll() {
  ResourceManager::clear();
}

void StorageManager::printResources() const {
  std::cout << "======= Resources =======" << std::endl;
  for (const auto &kv : all()) {
    const auto &name = kv.first;
    const auto &resource = kv.second;
    if (auto table = std::dynamic_pointer_cast<storage::AbstractTable>(resource)) {
      std::cout << "Table "
                << table->size() << " rows "
                << table->columnCount() << " columns" << std::endl
                << "    Columns:";

      for (field_t i = 0; i != table->columnCount(); i++)
         std::cout << " " << table->metadataAt(i).getName();
    } else if (std::dynamic_pointer_cast<storage::AbstractIndex>(resource)) {
      std::cout << "Index " << name;
    } else {
      std::cout << "Unknown resource type " << name;
    }
    std::cout << std::endl;
  }
  std::cout << "====================" << std::endl;
}

void StorageManager::addInvertedIndex(std::string name, std::shared_ptr<storage::AbstractIndex> index) {
  add(name, index);
}

std::shared_ptr<storage::AbstractIndex> StorageManager::getInvertedIndex(std::string name) {
  return get<storage::AbstractIndex>(name);
}

} } // namespace hyrise::io

