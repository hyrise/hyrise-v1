// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_STORAGEMANAGER_H_
#define SRC_LIB_IO_STORAGEMANAGER_H_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "io/ResourceManager.h"

class AbstractTable;
class AbstractIndex;
class AbstractResource;

namespace Loader { class params; }                                            

namespace hyrise {
namespace io {

/// Central holder of schema information
class StorageManager : public ResourceManager {
 protected:
  StorageManager() = delete;
  StorageManager(const StorageManager &) = delete;
  StorageManager &operator= (const StorageManager &) = delete;

  /// Adds a new storage table to _schema, forwards to all constructors of StorageTable
  /// that start with (std::string name, ....)
  template<typename... Args>
  void addStorageTable(std::string name, Args&& ... args);

 public:
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

  /// Convenience loading from fileName
  /// @param[in] name Table name
  /// @param[in] fileName Path to to hyrise-format file
  void loadTableFile(std::string name, std::string fileName);

  /// Convenience loading for files with separate header
  /// @param[in] name Table name
  /// @param[in] dataFileName Path to to hyrise-format data file
  /// @param[in] headerFileName Path to to hyrise-format header file
  void loadTableFileWithHeader(std::string name, std::string dataFileName,
                               std::string headerFileName);
  /// Replace existing table
  /// @param[in] name Table name to be replaced
  /// @param[in] table Shared table pointer
  void replaceTable(std::string name, std::shared_ptr<AbstractTable> table);

  void removeTable(std::string name);

  void removeAll();

  /// Get a table
  /// @param[in] name Table name
  std::shared_ptr<AbstractTable> getTable(std::string name);

  /// saves the inverted index as name.
  void addInvertedIndex(std::string name, std::shared_ptr<AbstractIndex> _index);

  /// returns the index stored under name name.
  std::shared_ptr<AbstractIndex> getInvertedIndex(std::string name);

  /// Retrieve all table names
  std::vector<std::string> getTableNames() const;

  /// Prints all Resources
  void printResources() const;

  /// Get full path for fileName
  /// @param[in] fileName fileName
  std::string makePath(std::string fileName);
};

}
}  // namespace hyrise::io



typedef hyrise::io::StorageManager StorageManager;

#endif  // SRC_LIB_IO_STORAGEMANAGER_H_

