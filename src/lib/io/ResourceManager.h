// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <stdexcept>

#include "helper/types.h"
#include "helper/checked_cast.h"

class AbstractResource;

namespace hyrise {
namespace storage {
class AbstractRessource;
} // namespace storage

namespace io {

class ResourceManagerException :  public std::runtime_error {
 public:
  explicit ResourceManagerException(const std::string &what): std::runtime_error(what) {}
};

class ResourceAlreadyExistsException : public ResourceManagerException {
 public:
  explicit ResourceAlreadyExistsException(const std::string &what): ResourceManagerException(what) {}
};

class ResourceNotExistsException : public ResourceManagerException {
 public:
  explicit ResourceNotExistsException(const std::string &what): ResourceManagerException(what) {}
};

/// Manages AbstractResources by name
///
/// @note
/// All methods on this class are `const` in the C++11 sense of
/// being thread-safe since all interactions with the resource manager
/// *need* to be thread-safe.
class ResourceManager {
 public:
  typedef std::map<std::string, std::shared_ptr<storage::AbstractResource> > resource_map;
  /// Retrieve singleton ResourceManager instance
  static ResourceManager& getInstance();

  /// Adds a new resource
  void add(const std::string& name, const std::shared_ptr<storage::AbstractResource>& resource) const;

  /// Retrieves a resource by name
  std::shared_ptr<storage::AbstractResource> getResource(const std::string& name) const;

  /// Retrieves a resource by name and assures its type T
  template <typename T>
  std::shared_ptr<T> get(const std::string& name) const {
    return checked_pointer_cast<T>(getResource(name));
  }

  /// Removes a named resource
  void remove(const std::string& name) const;

  /// Replaces an existing resource
  void replace(const std::string& name, const std::shared_ptr<storage::AbstractResource>& resource) const;
  
  /// Removes all resources
  void clear() const;

  /// Test for resource existance
  /// @param[in] name Resource name
  bool exists(const std::string& name) const;

  /// Test for resource existance, throws ResourceManagerException
  /// @param[in] name Resource name
  void assureExists(const std::string& name) const;

  /// Return number of elements in storage
  size_t size() const;
  
  /// Get a copy of the full resource map, otherwise, we would need to
  /// lock the whole structure while other operations are running on
  /// top of it
  resource_map all() const;
 private:
  /// The actual schema
  mutable resource_map _resources;
  /// Mutex protecting the _schema map
  mutable std::recursive_mutex _resource_mutex;

  ResourceManager() = default;
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator= (const ResourceManager &) = delete;

};

}}  // namespace hyrise::io

