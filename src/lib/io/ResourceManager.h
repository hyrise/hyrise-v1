// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_RESOURCEMANAGER_H_
#define SRC_LIB_IO_RESOURCEMANAGER_H_

#include <mutex>

#include <helper/types.h>

class AbstractResource;
class AbstractTable;
class AbstractIndex;

namespace hyrise {
namespace io {

class ResourceManagerException :  public std::runtime_error {
 public:
  explicit ResourceManagerException(const std::string &what): std::runtime_error(what) {}
};

class AlreadyExistsException : public ResourceManagerException {
 public:
  explicit AlreadyExistsException(const std::string &what): ResourceManagerException(what) {}
};

/// Central holder of schema information
class ResourceManager {
 public:
  typedef std::map<std::string, std::shared_ptr<AbstractResource> > resource_map;

 protected:
  /// The actual schema
  resource_map _resources;
  /// Mutex protecting the _schema map
  std::mutex _resource_mutex;

  /// Base path for loading
  //std::string _root_path;
  /// Assures that we only initialize once
  //bool _initialized;

  //typedef std::map<std::string, std::shared_ptr<AbstractIndex>> indices_t;
  /// Indices map
  //indices_t _indices;

  ResourceManager();
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator= (const ResourceManager &) = delete;

  /// Create all systems base tables require to run
  void setupSystem();
  /// unloads all tables
  //TODO void unloadAll();

 public:
  ~ResourceManager();

  /// Retrieve singleton ResourceManager instance
  //TODO static ResourceManager *getInstance();
  
  void clear();

  template <typename T>
  void add(std::string name, std::shared_ptr<T> resource);

  template <typename T = AbstractResource>
  std::shared_ptr<T> get(std::string name) const;

  /// Test for resource existance
  /// @param[in] name Resource name
  template <typename T = AbstractResource>
  bool exists(std::string name) const;

  /// Test for resource existance, throws ResourceManagerException
  /// @param[in] name Resource name
  template <typename T = AbstractResource>
  void assureExists(std::string name) const;
};

}
}  // namespace hyrise::io

typedef hyrise::io::ResourceManager ResourceManager;

#endif  // SRC_LIB_IO_RESOURCEMANAGER_H_

