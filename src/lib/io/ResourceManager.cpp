// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/ResourceManager.h"

#include "helper/stringhelpers.h"
#include "helper/Environment.h"
#include "storage/AbstractResource.h"

namespace hyrise {
namespace io {

ResourceManager& ResourceManager::getInstance() {
  static ResourceManager rm;
  return rm;
}

template<typename T>
std::unique_lock<T> lock_guard(T& mtx) {
  return std::move(std::unique_lock<T>(mtx));
}

size_t ResourceManager::size() const {
  auto lock = lock_guard(_resource_mutex) ;
  return _resources.size();
}

bool ResourceManager::exists(const std::string& name) const {
  auto lock = lock_guard(_resource_mutex) ;
  return _resources.count(name) == 1;
}

void ResourceManager::assureExists(const std::string& name) const {
  auto lock = lock_guard(_resource_mutex) ;
  if (!exists(name)) {
    throw ResourceNotExistsException("ResourceManager: Resource \'" + name + "\' does not exist");
  }
}

void ResourceManager::clear() const {
  auto lock = lock_guard(_resource_mutex) ;
  _resources.clear();
}

void ResourceManager::remove(const std::string& name) const {
  auto lock = lock_guard(_resource_mutex) ;
  assureExists(name);
  _resources.erase(name);
}

void ResourceManager::replace(const std::string& name, const  std::shared_ptr<storage::AbstractResource>& resource) const {
  auto lock = lock_guard(_resource_mutex) ;
  assureExists(name);
  _resources.at(name) = resource;
}

void ResourceManager::add(const std::string& name, const std::shared_ptr<storage::AbstractResource>& resource) const {
  auto lock = lock_guard(_resource_mutex) ;
  if (exists(name))
    throw ResourceAlreadyExistsException("ResourceManager: Resource '" + name + "' already exists");
  _resources.insert(make_pair(name, resource));
}

std::shared_ptr<storage::AbstractResource> ResourceManager::getResource(const std::string& name) const {
  auto lock = lock_guard(_resource_mutex) ;
  assureExists(name);
  return _resources.at(name);
}

ResourceManager::resource_map ResourceManager::all() const {
  auto lock = lock_guard(_resource_mutex);
  return _resources;
}

} } // namespace hyrise::io

