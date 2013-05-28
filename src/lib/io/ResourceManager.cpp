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
#include "storage/AbstractTable.h"
#include "storage/AbstractIndex.h"
#include "storage/TableBuilder.h"
#include "io/CSVLoader.h"

namespace hyrise {
namespace io {

typedef ResourceManager::resource_map::value_type resource_pair;

const std::string indexPrefix = "index:";

//const char ResourceManager::SYS_STATISTICS[] = "sys:statistics";

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {}

/*std::shared_ptr<AbstractTable>  ResourceManager::buildStatisticsTable() {
  // Prepare statistics table
  TableBuilder::param_list list;
  list.append().set_type("STRING").set_name("query_id");
  list.append().set_type("STRING").set_name("operator");
  list.append().set_type("STRING").set_name("detail");
  list.append().set_type("FLOAT").set_name("selectivity");
  list.append().set_type("INTEGER").set_name("start");
  return TableBuilder::build(list);
}*/

void ResourceManager::setupSystem() {
/*  static std::mutex instance_mtx;
  std::lock_guard<std::mutex> lock(instance_mtx);
//no unlock?
  if (!_initialized) {
    _root_path = getEnv("HYRISE_DB_PATH", "");
    // add the statistics table
    addStorageTable(SYS_STATISTICS, buildStatisticsTable());
    _initialized = true;
  }*/
}

/*TODO ResourceManager *ResourceManager::getInstance() {
  static StorageManager instance;
  instance.setupSystem();
  return &instance;
}*/

template <>
bool ResourceManager::exists<AbstractResource>(const std::string name) const {
  return _resources.count(name) >= 1;
}

template <>
bool ResourceManager::exists<AbstractTable>(const std::string name) const {
  return (exists<AbstractResource>(name) &&
         std::dynamic_pointer_cast<AbstractTable>(_resources.at(name)) != nullptr);
}

template <>
bool ResourceManager::exists<AbstractIndex>(const std::string name) const {
  const std::string indexName = indexPrefix + name;
  return (exists<AbstractResource>(indexName) &&
         std::dynamic_pointer_cast<AbstractIndex>(_resources.at(indexName)) != nullptr);
}

template <>
void ResourceManager::assureExists<AbstractResource>(std::string name) const {
  if (!exists<AbstractResource>(name))
    throw ResourceManagerException("ResourceManager: Resource '" + name + "' does not exist");
}

template <>
void ResourceManager::assureExists<AbstractTable>(std::string name) const {
  if (!exists<AbstractTable>(name))
    throw ResourceManagerException("ResourceManager: Table '" + name + "' does not exist");
}

template <>
void ResourceManager::assureExists<AbstractIndex>(std::string name) const {
  if (!exists<AbstractIndex>(name))
    throw ResourceManagerException("ResourceManager: Index '" + name + "' does not exist");
}

void ResourceManager::clear() {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  _resources.clear();
}

template <>
void ResourceManager::add<AbstractTable>(const std::string name, std::shared_ptr<AbstractTable> resource)
{
  if (!name.compare(0, indexPrefix.size(), indexPrefix))
    throw ResourceManagerException("ResourceManager: Table names may not begin with '" + indexPrefix + "'");
  if (exists<AbstractTable>(name))
    throw AlreadyExistsException("ResourceManager: Table '" + name + "' already exists");

  _resources.insert(resource_pair(name, resource));
}

template <>
void ResourceManager::add<AbstractIndex>(const std::string name, std::shared_ptr<AbstractIndex> resource)
{
  const std::string indexName = indexPrefix + name;

  //is it allowed to have an index without a table?
  if (exists<AbstractIndex>(name)) //or override
    throw AlreadyExistsException("ResourceManager: Index '" + name + "' already exists");

  _resources.insert(resource_pair(indexName, resource));
}

template <>
std::shared_ptr<AbstractResource> ResourceManager::get<AbstractResource>(const std::string name) const {
  assureExists<AbstractResource>(name);
  return _resources.at(name);
}

template <>
std::shared_ptr<AbstractTable> ResourceManager::get<AbstractTable>(const std::string name) const {
  assureExists<AbstractTable>(name);
  return std::dynamic_pointer_cast<AbstractTable>(_resources.at(name));
} 

template <>
std::shared_ptr<AbstractIndex> ResourceManager::get<AbstractIndex>(const std::string name) const {
  assureExists<AbstractIndex>(name);
  return std::dynamic_pointer_cast<AbstractIndex>(_resources.at(name));
}

}
} // namespace hyrise::io

