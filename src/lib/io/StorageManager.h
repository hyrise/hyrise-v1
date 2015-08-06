// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "io/ResourceManager.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace hyrise {

namespace storage {
class AbstractTable;
class AbstractIndex;
class AbstractResource;
}  // namespace storage

namespace io {
namespace Loader {
class params;
}  // namespace Loader

/// Central holder of schema information
class StorageManager : public ResourceManager {
 protected:
  StorageManager() = delete;
  StorageManager(const StorageManager&) = delete;
  StorageManager& operator=(const StorageManager&) = delete;

  /// Adds a new storage table to _schema, forwards to all constructors of StorageTable
  /// that start with (std::string name, ....)
  template <typename... Args>
  void addStorageTable(const std::string& name, Args&&... args);

 public:
  /// Retrieve singleton storage-manager instance
  static StorageManager* getInstance();

  /// Table loading with parameters
  /// @param[in] name Table name
  /// @param[in] parameters Loader parameters for delayed load
  void loadTable(const std::string& name, const Loader::params& parameters, const std::string path = "");

  /// Table loading with table
  /// @param[in] name Table name
  /// @param[in] table Shared table pointer
  void loadTable(const std::string& name, std::shared_ptr<storage::AbstractTable> table);

  /// Convenience loading from fileName
  /// @param[in] name Table name
  /// @param[in] fileName Path to to hyrise-format file
  void loadTableFile(const std::string& name, std::string fileName, const std::string path = "");

  /// Convenience loading for files with separate header
  /// @param[in] name Table name
  /// @param[in] dataFileName Path to to hyrise-format data file
  /// @param[in] headerFileName Path to to hyrise-format header file
  void loadTableFileWithHeader(const std::string& name,
                               const std::string& dataFileName,
                               const std::string& headerFileName);
  /// Replace existing table
  /// @param[in] name Table name to be replaced
  /// @param[in] table Shared table pointer
  void replaceTable(const std::string& name, std::shared_ptr<storage::AbstractTable> table);

  void removeTable(const std::string& name);

  void removeAll();

  /// Get a table
  /// @param[in] name Table name
  std::shared_ptr<storage::AbstractTable> getTable(const std::string& name, bool unsafe = false);

  /// saves the inverted index as name.
  void addInvertedIndex(const std::string& name, std::shared_ptr<storage::AbstractIndex> _index);

  /// returns the index stored under name name.
  std::shared_ptr<storage::AbstractIndex> getInvertedIndex(const std::string& name, bool unsafe = false);

  /// Retrieve all table names
  std::vector<std::string> getTableNames() const;

  /// Prints all Resources
  void printResources() const;

  /// Get full path for fileName
  /// @param[in] fileName fileName
  std::string makePath(const std::string& fileName);

  void persistTable(const std::string& name, std::string path = "");

  void recoverTables(const size_t thread_count = 1);

  void recoverTable(const std::string& name, std::string path = "", const size_t thread_count = 1);

  void recoverCheckpoint(const std::string& checkpoint_dir, const std::string& filename);
};
}
}  // namespace hyrise::io
