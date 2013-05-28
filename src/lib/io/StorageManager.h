// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_STORAGEMANAGER_H_
#define SRC_LIB_IO_STORAGEMANAGER_H_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <io/Loader.h>
#include <io/ResourceManager.h>

class AbstractResource;
class AbstractTable;
class AbstractIndex;

namespace hyrise {
namespace io {

class MissingIndexException : public ResourceManagerException {
 public:
  explicit MissingIndexException(const std::string &what): ResourceManagerException(what) {}
};

/// Storage class that holds tables that might not be loaded yet
class StorageTable {
 private:
  std::string _name;
  std::shared_ptr<AbstractTable> _table;
  std::unique_ptr<Loader::params> _parameters;
  std::mutex _table_mutex;

 public:
  StorageTable();
  explicit StorageTable(std::string table_name);
  StorageTable(std::string table_name, std::shared_ptr<AbstractTable> table);
  StorageTable(std::string table_name, const Loader::params &_parameters);
  StorageTable(const StorageTable &st);
  ~StorageTable();

  void load();
  void unload();

  bool isLoaded() const;
  bool isLoadable() const;

  std::shared_ptr<AbstractTable> getTable();
  std::shared_ptr<AbstractTable> getTable() const;
  void setTable(std::shared_ptr<AbstractTable> table);
};

/// Central holder of schema information
class StorageManager : ResourceManager {
 protected:
  /// The actual schema
  std::map<std::string, StorageTable> _schema;
  /// Mutex protecting the _schema map
  std::mutex _schema_mutex;
  /// Base path for loading
  std::string _root_path;
  /// Assures that we only initialize once
  bool _initialized;

  typedef std::map<std::string, std::shared_ptr<AbstractIndex>> indices_t;
  /// Indices map
  indices_t _indices;

  StorageManager();
  StorageManager(const StorageManager &) = delete;
  StorageManager &operator= (const StorageManager &) = delete;

  /// Adds a new storage table to _schema, forwards to all constructors of StorageTable
  /// that start with (std::string name, ....)
  template<typename... Args>
  void addStorageTable(std::string name, Args && ... args) {
    std::lock_guard<std::mutex> lock(_schema_mutex);
    if (_schema.count(name) != 0) {
      throw AlreadyExistsException("'" + name + "' is already in schema");
    };
    _schema.insert(make_pair(name, StorageTable(name,
                                                std::forward<Args>(args)...)));
  }

  /// Create all systems base tables require to run
  void setupSystem();
  /// unloads all tables
  void unloadAll();

  static std::vector<std::string> listDirectory(std::string dir);

 public:
  typedef std::map<std::string, StorageTable> schema_map_t;

  ~StorageManager();

  /// Retrieve singleton storage-manager instance
  static StorageManager *getInstance();

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

  void preloadTable(std::string name);
  void unloadTable(std::string name);
  void removeTable(std::string name);

  void removeAll();
  void preloadAll();

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
  size_t size() const; //TODO remove

  /// Prints the complete schema
  void printSchema() const;

  /// Get full path for filename
  /// @param[in] filename filename
  std::string makePath(std::string filename);

  /// Creates an empty statistics table as used for the general
  ///  statistics of the complete system
  static std::shared_ptr<AbstractTable> buildStatisticsTable();

  static const char SYS_STATISTICS[];
};
}
}  // namespace hyrise::io

typedef hyrise::io::StorageManager StorageManager;

#endif  // SRC_LIB_IO_STORAGEMANAGER_H_

