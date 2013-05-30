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

  ResourceManager();
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator= (const ResourceManager &) = delete;

 public:
  ~ResourceManager();

  /// Retrieve singleton ResourceManager instance
  static ResourceManager *getInstance();
  
  void clear();

  void remove(std::string name);

  void replace(std::string name, std::shared_ptr<AbstractResource> resource);

  void add(std::string name, std::shared_ptr<AbstractResource> resource);

  std::shared_ptr<AbstractResource> get(std::string name);

  /// Test for resource existance
  /// @param[in] name Resource name
  bool exists(std::string name) const;

  /// Test for resource existance, throws ResourceManagerException
  /// @param[in] name Resource name
  void assureExists(std::string name) const;
};

}
}  // namespace hyrise::io

typedef hyrise::io::ResourceManager ResourceManager;

#endif  // SRC_LIB_IO_RESOURCEMANAGER_H_

