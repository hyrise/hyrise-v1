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

#include "CSVLoader.h"
#include "StorageManager.h"

namespace hyrise {
namespace io {

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

ResourceManager* ResourceManager::getInstance() {
  return StorageManager::getInstance();
}

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
void ResourceManager::remove<AbstractResource>(const std::string name) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists<AbstractResource>(name);
  
  _resources.erase(name);
}

template <>
void ResourceManager::remove<AbstractTable>(const std::string name) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists<AbstractTable>(name);
  
  _resources.erase(name);
}

template <>
void ResourceManager::remove<AbstractIndex>(const std::string name) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists<AbstractIndex>(name);
  
  const std::string indexName = indexPrefix + name;
  _resources.erase(indexName);
}

template <>
void ResourceManager::replace<AbstractTable>(const std::string name, std::shared_ptr<AbstractTable> resource) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists<AbstractTable>(name);
  
  _resources.at(name) = resource;
}

template <>
void ResourceManager::replace<AbstractIndex>(const std::string name, std::shared_ptr<AbstractIndex> resource) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists<AbstractIndex>(name);
  
  const std::string indexName = indexPrefix + name;
  _resources.erase(indexName);
}

template <>
void ResourceManager::add<AbstractTable>(const std::string name, std::shared_ptr<AbstractTable> resource) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  
  if (!name.compare(0, indexPrefix.size(), indexPrefix))
    throw ResourceManagerException("ResourceManager: Table names may not begin with '" + indexPrefix + "'");
  if (exists<AbstractTable>(name))
    throw AlreadyExistsException("ResourceManager: Table '" + name + "' already exists");

  _resources.insert(make_pair(name, resource));
}

template <>
void ResourceManager::add<AbstractIndex>(const std::string name, std::shared_ptr<AbstractIndex> resource) {
  std::lock_guard<std::mutex> lock(_resource_mutex);

  //is it allowed to have an index without a table?
  if (exists<AbstractIndex>(name)) //or override
    throw AlreadyExistsException("ResourceManager: Index '" + name + "' already exists");

  const std::string indexName = indexPrefix + name;
  _resources.insert(make_pair(indexName, resource));
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
  const std::string indexName = indexPrefix + name;
  return std::dynamic_pointer_cast<AbstractIndex>(_resources.at(indexName));
}

}
} // namespace hyrise::io

