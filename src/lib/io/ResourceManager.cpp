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
#include "storage/AbstractResource.h"

#include "CSVLoader.h"
#include "StorageManager.h"

namespace hyrise {
namespace io {

const std::string indexPrefix = "index:";

//const char ResourceManager::SYS_STATISTICS[] = "sys:statistics";

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {}

ResourceManager* ResourceManager::getInstance() {
  return StorageManager::getInstance();
}

bool ResourceManager::exists(const std::string name) const {
  return _resources.count(name) >= 1;
}

void ResourceManager::assureExists(std::string name) const {
  if (!exists(name))
    throw ResourceManagerException("ResourceManager: Resource '" + name + "' does not exist");
}

void ResourceManager::clear() {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  _resources.clear();
}

void ResourceManager::remove(const std::string name) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists(name);
  
  _resources.erase(name);
}

void ResourceManager::replace(const std::string name, std::shared_ptr<AbstractResource> resource) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists(name);
  
  _resources.at(name) = resource;
}

void ResourceManager::add(const std::string name, std::shared_ptr<AbstractResource> resource) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  
  if (exists(name))
    throw AlreadyExistsException("ResourceManager: Resource '" + name + "' already exists");

  _resources.insert(make_pair(name, resource));
}

std::shared_ptr<AbstractResource> ResourceManager::get(const std::string name) {
  std::lock_guard<std::mutex> lock(_resource_mutex);
  assureExists(name);
  
  return _resources.at(name);
}

}
} // namespace hyrise::io

