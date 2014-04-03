// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/StorageManager.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <dirent.h>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "helper/Settings.h"
#include "helper/stringhelpers.h"
#include "helper/Environment.h"
#include "helper/dir.h"
#include "io/Loader.h"
#include "io/CSVLoader.h"
#include "io/TableDump.h"
#include "storage/AbstractIndex.h"
#include "storage/AbstractTable.h"
#include "storage/ColumnMetadata.h"
#include "storage/DictionaryFactory.h"
#include "storage/TableBuilder.h"
#include "storage/Store.h"
#include "storage/DeltaIndex.h"
#include "storage/GroupkeyIndex.h"

#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/iter_find.hpp>


namespace hyrise {
namespace io {

template <typename... Args>
void StorageManager::addStorageTable(const std::string& name, Args&&... args) {
  add(name, Loader::load(std::forward<Args>(args)...));
}

StorageManager* StorageManager::getInstance() {
  // TODO: This is a hack for the moment, since StorageManager only adds to the interface
  // but does not have its own members. This keeps the old interface intact until we migrate
  // remaining code.
  return static_cast<StorageManager*>(&ResourceManager::getInstance());
}

void StorageManager::loadTable(const std::string& name, std::shared_ptr<storage::AbstractTable> table) {
  add(name, table);
}

void StorageManager::replaceTable(const std::string& name, std::shared_ptr<storage::AbstractTable> table) {
  replace(name, table);
}

void StorageManager::loadTable(const std::string& name, const Loader::params& parameters, const std::string path) {
  Loader::params* p = parameters.clone();
  if (path.empty()) {
    p->setBasePath(Settings::getInstance()->getDBPath() + "/");
  } else {
    p->setBasePath(path + "/");
  }
  addStorageTable(name, *p);
  delete p;
}

void StorageManager::loadTableFile(const std::string& name, std::string fileName, const std::string path) {
  std::string inputfile = "";
  if (path.empty()) {
    inputfile = makePath(fileName);
  } else {
    inputfile = path + "/" + fileName;
  }
  CSVInput input(inputfile);
  CSVHeader header(inputfile);
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  p.setTableName(name);
  addStorageTable(name, p);
}

void StorageManager::loadTableFileWithHeader(const std::string& name,
                                             const std::string& datafileName,
                                             const std::string& headerFileName) {
  CSVInput input(makePath(datafileName));
  CSVHeader header(makePath(headerFileName));
  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  p.setTableName(name);
  addStorageTable(name, p);
}

std::string StorageManager::makePath(const std::string& fileName) {
  return Settings::getInstance()->getDBPath() + "/" + fileName;
}

std::shared_ptr<storage::AbstractTable> StorageManager::getTable(const std::string& name, bool unsafe) {
  if (!exists(name, unsafe)) {
    std::string tbl_file = Settings::getInstance()->getDBPath() + "/" + name + ".tbl";
    struct stat stFileInfo;

    if (stat(tbl_file.c_str(), &stFileInfo) == 0)
      loadTableFile(name, name + ".tbl");
  }
  return get<storage::AbstractTable>(name, unsafe);
}

void StorageManager::removeTable(const std::string& name) {
  if (exists(name))
    remove(name);
}

std::vector<std::string> StorageManager::getTableNames() const {
  std::vector<std::string> ret;
  for (const auto& resource : all())
    if (std::dynamic_pointer_cast<storage::AbstractTable>(resource.second) != nullptr)
      ret.push_back(resource.first);
  return ret;
}

void StorageManager::removeAll() { ResourceManager::clear(); }

void StorageManager::printResources() const {
  std::cout << "======= Resources =======" << std::endl;
  for (const auto& kv : all()) {
    const auto& name = kv.first;
    const auto& resource = kv.second;
    if (auto table = std::dynamic_pointer_cast<storage::AbstractTable>(resource)) {
      std::cout << "Table " << table->size() << " rows " << table->columnCount() << " columns" << std::endl
                << "    Columns:";

      for (field_t i = 0; i != table->columnCount(); i++)
        std::cout << " " << table->metadataAt(i).getName();
    } else if (std::dynamic_pointer_cast<storage::AbstractIndex>(resource)) {
      std::cout << "Index " << name;
    } else {
      std::cout << "Unknown resource type " << name;
    }
    std::cout << std::endl;
  }
  std::cout << "====================" << std::endl;
}

void StorageManager::addInvertedIndex(const std::string& name, std::shared_ptr<storage::AbstractIndex> index) {
  add(name, index);
}

std::shared_ptr<storage::AbstractIndex> StorageManager::getInvertedIndex(const std::string& name, bool unsafe) {
  return get<storage::AbstractIndex>(name, unsafe);
}

void StorageManager::persistTable(const std::string& name, std::string path) {
  if (!exists(name)) {
    throw std::runtime_error("Cannot persist nonexisting table");
  }

  auto table = getTable(name);

  if (path == "")
    path = Settings::getInstance()->getTableDumpDir();

  storage::SimpleTableDump td(path);

  auto store = std::dynamic_pointer_cast<storage::Store>(table);
  if (store) {
    store->enableLogging();
  }

  td.dump(name, table);
}


#ifdef PERSISTENCY_BUFFEREDLOGGER
void StorageManager::recoverTables(const size_t thread_count) {

  // load table dumps containing the main
  auto table_files = _listdir(Settings::getInstance()->getTableDumpDir());
  std::vector<std::thread> threadpool;
  for (auto filename : table_files) {
    if (filename[0] != '.') {
      threadpool.push_back(std::thread(&StorageManager::recoverTable, this, filename, "", thread_count));
    }
  }
  for (auto& t : threadpool) {
    t.join();
  }
  threadpool.clear();

  // load delta from last checkpoint
  auto last_checkpoint = BufferedLogger::getInstance().getLastCheckpointID();
  auto checkpoint_dir = Settings::getInstance()->getCheckpointDir() + "/" + std::to_string(last_checkpoint) + "/";
  auto checkpoint_files = _listdir(checkpoint_dir);
  for (auto filename : checkpoint_files) {
    if (filename.at(0) != '.') {
      threadpool.push_back(std::thread(&StorageManager::recoverCheckpoint, this, checkpoint_dir, filename));
    }
  }
  for (auto& t : threadpool) {
    t.join();
  }

  // replay last delta log
  BufferedLogger::getInstance().restore(thread_count);
}
#endif

void StorageManager::recoverCheckpoint(const std::string& checkpoint_dir, const std::string& filename) {
  auto table = getTable(filename);
  auto store = std::dynamic_pointer_cast<storage::Store>(table);
  io::TableDumpLoader loader(checkpoint_dir, filename);
  CSVHeader header(checkpoint_dir + "/" + filename + "/header.dat",
                   CSVHeader::params().setCSVParams(csv::HYRISE_FORMAT));
  auto delta = Loader::load(
      Loader::params().setInput(loader).setHeader(header).setDeltaDataStructure(true).setModifiableMutableVerticalTable(
          true));
  store->setDelta(delta);
  loader.loadCidVectors(filename, store);
}

#ifdef PERSISTENCY_NONE
void StorageManager::recoverTables(const size_t thread_count) {
  std::cout << "Running without any persistency support. Cannot recover tables." << std::endl;
  // throw std::runtime_error("Recovery not supported");
}
#endif

void StorageManager::recoverTable(const std::string& name, std::string path, const size_t thread_count) {

  if (path == "")
    path = Settings::getInstance()->getTableDumpDir();

  io::TableDumpLoader loader(path, name);
  CSVHeader header(path + "/" + name + "/header.dat", CSVHeader::params().setCSVParams(csv::HYRISE_FORMAT));
  auto t = Loader::load(Loader::params().setInput(loader).setHeader(header));
  t->setName(name);
  loader.loadIndices(t);

  if (exists(name)) {
    throw std::runtime_error("cannot recover already loaded table");
  }

  add(name, t);
}
}
}  // namespace hyrise::io
