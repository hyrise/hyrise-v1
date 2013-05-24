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
#include "storage/TableBuilder.h"
#include "io/CSVLoader.h"

namespace hyrise {
namespace io {

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

bool ResourceManager::exists(std::string name) const {
  return _resources.count(name) >= 1;
}

void ResourceManager::assureExists(std::string name) const {
  if (!exists(name)) throw ResourceManagerException("Table '" + name + "' does not exist");
}

namespace {
const std::string SYSTEM_PREFIX = "sys:";

bool is_user_resource(ResourceManager::resource_map::value_type i) {
  return !(i.first.compare(0, SYSTEM_PREFIX.size(), SYSTEM_PREFIX) == 0);
}

bool is_user_table(ResourceManager::resource_map::value_type i) {
  //it still need a check whether this is a table or an index
  return !(i.first.compare(0, SYSTEM_PREFIX.size(), SYSTEM_PREFIX) == 0);
}

bool is_user_index(ResourceManager::resource_map::value_type i) {
  //it still need a check whether this is a table or an index
  return !(i.first.compare(0, SYSTEM_PREFIX.size(), SYSTEM_PREFIX) == 0);
}
} // namespace

std::vector<std::string> ResourceManager::getResourceNames() const {
  std::vector<std::string> ret;
  for (auto i = _resources.begin(); i != _resources.cend(); ++i) {
    if (is_user_resource(*i))
      ret.push_back((*i).first);
  }
  return ret;
}

size_t ResourceManager::numberOfResources() const {
  return std::count_if(_resources.cbegin(), _resources.cend(), is_user_resource);
}

std::vector<std::string> ResourceManager::getTableNames() const {
  std::vector<std::string> ret;
  for (auto i = _resources.begin(); i != _resources.cend(); ++i) {
    if (is_user_table(*i))
      ret.push_back((*i).first);
  }
  return ret;
}

size_t ResourceManager::numberOfTables() const {
  return std::count_if(_resources.cbegin(), _resources.cend(), is_user_table);
}

std::vector<std::string> ResourceManager::getIndexNames() const {
  std::vector<std::string> ret;
  for (auto i = _resources.begin(); i != _resources.cend(); ++i) {
    if (is_user_index(*i))
      ret.push_back((*i).first);
  }
  return ret;
}

size_t ResourceManager::numberOfIndices() const {
  return std::count_if(_resources.cbegin(), _resources.cend(), is_user_index);
}

void ResourceManager::clear() {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  _resources.clear();
}

}
} // namespace hyrise::io

