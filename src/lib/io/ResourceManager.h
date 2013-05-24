// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_RESOURCEMANAGER_H_
#define SRC_LIB_IO_RESOURCEMANAGER_H_

#include <mutex>

class AbstractResource;
class AbstractTable;
class AbstractIndex;

namespace hyrise {
namespace io {

class ResourceManagerException :  public std::runtime_error {
 public:
    explicit ResourceManagerException(const std::string &what): std::runtime_error(what) {}
};

class WrongTypeException : public ResourceManagerException {
 public:
  explicit WrongTypeException(const std::string &what): ResourceManagerException(what) {}
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
  T get(std::string name) { return T(); /*TODO*/}

  /// Test for resource existance
  /// @param[in] name Table name
  bool exists(std::string name) const;

  /// Test for resource existance, throws std::exception
  /// @param[in] name Table name
  void assureExists(std::string name) const;

  /// Retrieve all index names
  std::vector<std::string> getResourceNames() const;
  /// Retrieve number of indices
  size_t numberOfResources() const;

  /// Retrieve all table names
  std::vector<std::string> getTableNames() const;
  /// Retrieve number of tables
  size_t numberOfTables() const;

  /// Retrieve all index names
  std::vector<std::string> getIndexNames() const;
  /// Retrieve number of indices
  size_t numberOfIndices() const;
};

}
}  // namespace hyrise::io

typedef hyrise::io::ResourceManager ResourceManager;

#endif  // SRC_LIB_IO_RESOURCEMANAGER_H_

