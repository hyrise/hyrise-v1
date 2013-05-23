// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_RESOURCEMANAGER_H_
#define SRC_LIB_IO_RESOURCEMANAGER_H_

#include <io/StorageManager.h>

class AbstractResource;
class AbstractTable;
class AbstractIndex;

namespace hyrise {
namespace io {

/*class StorageManagerException : public std::runtime_error {
 public:
  explicit StorageManagerException(const std::string &what): std::runtime_error(what) {}
};

class AlreadyExistsException : public StorageManagerException {
 public:
  explicit AlreadyExistsException(const std::string &what): StorageManagerException(what) {}
};

class MissingIndexException : public StorageManagerException {
 public:
  explicit MissingIndexException(const std::string &what): StorageManagerException(what) {}
};*/

class WrongTypeException : public StorageManagerException {
 public:
  explicit WrongTypeException(const std::string &what): StorageManagerException(what) {}
};

/// Central holder of schema information
class ResourceManager : public StorageManager {
 private:
  /// The actual schema
  typedef std::map<std::string, std::shared_ptr<AbstractResource> > resource_map;
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

  /// Adds a new storage table to _schema, forwards to all constructors of StorageTable
  /// that start with (std::string name, ....)
  template<typename... Args>
  void addStorageTable(std::string name, Args && ... args) {
    std::lock_guard<std::mutex> lock(_schema_mutex);
    if (_schema.count(name) != 0) {
      throw AlreadyExistsException("'" + name + "' is already in schema");
    };
    //TODO
    //_schema.insert(make_pair(name, StorageTable(name,
    //                                            std::forward<Args>(args)...)));
  }

  /// Create all systems base tables require to run
  void setupSystem();
  /// unloads all tables
  //TODO void unloadAll();

 public:
  ~ResourceManager();

  /// Retrieve singleton storage-manager instance
  static ResourceManager *getInstance();

  /// Table loading with parameters
  /// @param[in] name Table name
  /// @param[in] parameters Loader parameters for delayed load
  void loadTable(std::string name, const Loader::params &parameters);

  /// Table loading with table
  /// @param[in] name Table name
  /// @param[in] table Shared table pointer
  void loadTable(std::string name, std::shared_ptr<AbstractTable> table);

  /// Convenience loading from filename
  /// @param[in] name Table name
  /// @param[in] filename Path to to hyrise-format file
  void loadTableFile(std::string name, std::string filename);

  /// Convenience loading for files with separate header
  /// @param[in] name Table name
  /// @param[in] datafilename Path to to hyrise-format data file
  /// @param[in] headerfilename Path to to hyrise-format header file
  void loadTableFileWithHeader(std::string name, std::string datafilename,
                               std::string headerfilename);
  /// Replace existing table
  /// @param[in] name Table name to be replaced
  /// @param[in] table Shared table pointer
  void replaceTable(std::string name, std::shared_ptr<AbstractTable> table);

//TODO
/*  void preloadTable(std::string name);
  void unloadTable(std::string name);*/
  void removeTable(std::string name);

  void removeAll();
//TODO  void preloadAll();

  template <typename T>
  T get(std::string name) { return T(); /*TODO*/}

  /// Get a table
  /// @param[in] name Table name
  std::shared_ptr<AbstractTable> getTable(std::string name);

  /// saves the inverted index using the name table_name.
  void addInvertedIndex(std::string table_name, std::shared_ptr<AbstractIndex> _index);

  /// returns the index stored under name table_name.
  std::shared_ptr<AbstractIndex> getInvertedIndex(std::string table_name);

  /// Test for table existance
  /// @param[in] name Table name
  bool exists(std::string name) const;

  /// Test for table existance, throws std::exception
  /// @param[in] name Table name
  void assureExists(std::string name) const;

  /// Retrieve all table names
  std::vector<std::string> getTableNames() const;
  /// Retrieve number of tables
  size_t size() const;

  /// Prints the complete schema
  //TODO void printSchema() const;

  /// Get full path for filename
  /// @param[in] filename filename
  std::string makePath(std::string filename);

  /// Creates an empty statistics table as used for the general
  ///  statistics of the complete system
  static std::shared_ptr<AbstractTable> buildStatisticsTable();

  //static const char SYS_STATISTICS[];
};

}
}  // namespace hyrise::io

typedef hyrise::io::ResourceManager ResourceManager;

#endif  // SRC_LIB_IO_RESOURCEMANAGER_H_

