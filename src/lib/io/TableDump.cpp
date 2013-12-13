// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TableDump.h"

#include <errno.h>
#include <sys/stat.h>

#include <fstream>
#include <initializer_list>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "io/LoaderException.h"
#include "io/GenericCSV.h"
#include "io/CSVLoader.h"

#include "helper/stringhelpers.h"
#include "helper/vector_helpers.h"

#include "storage/AbstractTable.h"
#include "storage/Store.h"
#include "storage/storage_types.h"
#include "storage/storage_types_helper.h"
#include "storage/meta_storage.h"

namespace hyrise { namespace storage {

namespace DumpHelper {
  static const std::string META_DATA_EXT = "metadata.dat";
  static const std::string HEADER_EXT = "header.dat";
  static const std::string DICT_EXT = ".dict.dat";
  static const std::string ATTR_EXT = ".attr.dat";

  static inline std::string buildPath(std::initializer_list<std::string> l) {
    return functional::foldLeft(l, std::string(), infix("/"));
  }
}

/**
 * This functor is used to write directly a typed value to a stream
 */
struct write_to_stream_functor {
  typedef void value_type;
  std::ofstream& data;
  std::shared_ptr<AbstractTable> table;
  field_t col;
  value_id_t vid;

  write_to_stream_functor(std::ofstream& o, std::shared_ptr<AbstractTable> t):
      data(o), table(t), col(0), vid(0)
  {}

  inline void setVid(value_id_t v){ 
    vid = v;
  }

  inline void setCol(field_t c) {
    col = c;
  }

  template <typename R>
  inline void operator()(){
    data << table->getValueForValueId<R>(col, ValueId(vid, 0)) << "\n";
  }

};

/**
 * Helper functor used to read all values from a file and insert them
 * in sorted order into the dictionary. The functor uses a
 * boost::lexica_clast<T> for transformation from string to any other
 * value.
 */
struct write_to_dict_functor {
  typedef void value_type;

  std::ifstream& data;
  std::shared_ptr<AbstractTable> table;
  field_t col;

  write_to_dict_functor(std::ifstream& d, 
                        std::shared_ptr<AbstractTable> t,
                        field_t c):
      data(d), table(t), col(c){}


  template <typename R>
  inline void operator()(){
    auto map = std::dynamic_pointer_cast<BaseDictionary<R>>(table->dictionaryAt(col));
    std::string tmp;
    while (data.good()) {
      std::getline(data, tmp);
      if (!data.good() && tmp.size() == 0)
        break;
      map->addValue(boost::lexical_cast<R>(tmp));
      tmp.clear();
    }
  }

};

void SimpleTableDump::prepare(std::string name) {
  struct stat buffer;
  // Check if the directories exists and create if necessary with basic permissions
  if (stat(_baseDirectory.c_str(), &buffer) != 0)
    if (mkdir(_baseDirectory.c_str(), 0755) != 0 && errno != EEXIST)
      throw std::runtime_error(strerror(errno));

  std::string fullPath = _baseDirectory + "/" + name;
  if (stat(fullPath.c_str(), &buffer) != 0)
    if (mkdir(fullPath.c_str(), 0755) != 0 && errno != EEXIST)
      throw std::runtime_error(strerror(errno));
}

void SimpleTableDump::dumpDictionary(std::string name, std::shared_ptr<AbstractTable> table, size_t col) {
  std::string fullPath = _baseDirectory + "/" + name + "/" + table->nameOfColumn(col) + ".dict.dat";
  std::ofstream data (fullPath, std::ios::out | std::ios::binary);

  // We make a small hack here, first we obtain the size of the
  // dictionary then we virtually create all value ids, this can break
  // if the dictionary has no contigous value ids
  size_t dictionarySize = table->dictionaryAt(col)->size();
  write_to_stream_functor fun(data, table);
  type_switch<hyrise_basic_types> ts;
  for(size_t i=0; i < dictionarySize; ++i) {
    fun.setCol(col);
    fun.setVid(i);
    ts(table->typeOfColumn(col), fun);
  }
  data.close();
}

void SimpleTableDump::dumpAttribute(std::string name, std::shared_ptr<AbstractTable> table, size_t col) {
  std::string fullPath = _baseDirectory + "/" + name + "/" + table->nameOfColumn(col) + ".attr.dat";
  std::ofstream data (fullPath, std::ios::out | std::ios::binary);
  ValueId v;
  for(size_t i=0; i < table->size(); ++i) {
    v = table->getValueId(col, i);
    data.write((char*) &v.valueId, sizeof(v.valueId));
  }
  data.close();
}

void SimpleTableDump::dumpHeader(std::string name, std::shared_ptr<AbstractTable> table) {
  std::stringstream header;
  std::vector<std::string> names, types;
  std::vector<uint32_t> parts;

  // Get names and types
  for(size_t i=0; i < table->columnCount(); ++i) {
    names.push_back(table->nameOfColumn(i));
    types.push_back(data_type_to_string(table->typeOfColumn(i)));
  }

  // This calculation will break if the width of the value_id changes
  // or someone forgets to simply update the width accordingly in the
  // constructor of the table
  for(size_t i=0; i < table->partitionCount(); ++i) {
    parts.push_back(table->partitionWidth(i));
  }
    
  // Dump and join
  header << std::accumulate(names.begin(), names.end(), std::string(), infix(" | ")) << "\n";
  header << std::accumulate(types.begin(), types.end(), std::string(), infix(" | ")) << "\n";
  std::vector<std::string> allParts;
  for(size_t i=0; i < parts.size(); ++i) {
    auto p = parts[i];
    auto tmp = std::vector<std::string>(p, std::to_string(i) + "_R");
    allParts.insert(allParts.end(), tmp.begin(), tmp.end());
  }
  header << std::accumulate(allParts.begin(), allParts.end(), std::string(), infix(" | ")) << "\n";
  header << "===";

  std::string fullPath = _baseDirectory + "/" + name + "/header.dat";
  std::ofstream data (fullPath, std::ios::out | std::ios::binary);
  data << header.str();
  data.close();
}

void SimpleTableDump::dumpMetaData(std::string name, std::shared_ptr<AbstractTable> table) {
  std::string fullPath = _baseDirectory + "/" + name + "/metadata.dat";
  std::ofstream data (fullPath, std::ios::out | std::ios::binary);
  data << table->size();
  data.close();
}

void SimpleTableDump::verify(std::shared_ptr<AbstractTable> table) {
  auto res = std::dynamic_pointer_cast<Store>(table);
  if (!res) throw std::runtime_error("Can only dump Stores");

  if (res->subtableCount() <= 1) throw std::runtime_error("Store must have at least one main table");
  if (res->subtableCount() != 2) throw std::runtime_error("Multi-generation stores are not supported for dumping");
}

bool SimpleTableDump::dump(std::string name, std::shared_ptr<AbstractTable> table) {
  verify(table);
  auto mainTable = std::dynamic_pointer_cast<Store>(table)->getMainTable();
  prepare(name);
  for(size_t i=0; i < mainTable->columnCount(); ++i) {
    // For each attribute dump dictionary and values
    dumpDictionary(name, mainTable, i);
    dumpAttribute(name, mainTable, i);
  }

  dumpHeader(name, mainTable);
  dumpMetaData(name, mainTable);
  
  return true;
}

} // namespace storage

namespace io {

size_t TableDumpLoader::getSize() {
  std::string path = storage::DumpHelper::buildPath({_base, _table, storage::DumpHelper::META_DATA_EXT});
  std::ifstream data (path, std::ios::binary);
  size_t numRows;
  data >> numRows;
  data.close();
  return numRows;
}


void TableDumpLoader::loadDictionary(std::string name, size_t col, std::shared_ptr<storage::AbstractTable> intable) {
  std::string path = storage::DumpHelper::buildPath({_base, _table, name}) + storage::DumpHelper::DICT_EXT;
  std::ifstream data (path, std::ios::binary);

  storage::write_to_dict_functor fun(data, intable, col);
  storage::type_switch<hyrise_basic_types> ts;
  ts(intable->typeOfColumn(col), fun);

  data.close();
}

void TableDumpLoader::loadAttribute(std::string name, size_t col, size_t size, std::shared_ptr<storage::AbstractTable> intable) {
  std::string path = storage::DumpHelper::buildPath({_base, _table, name}) + storage::DumpHelper::ATTR_EXT;
  std::ifstream data (path, std::ios::binary);
  
  ValueId vid;
  for(size_t i=0; i < size; ++i) {
    data.read((char*) &vid.valueId, sizeof(vid.valueId));
    intable->setValueId(col, i, vid);
  }
  
  data.close();
}


std::shared_ptr<storage::AbstractTable> TableDumpLoader::load(std::shared_ptr<storage::AbstractTable> intable, 
                                                              const storage::compound_metadata_list *meta, 
                                                              const Loader::params &args)
{

  // First extract the dictionaries
  for(size_t i=0; i < intable->columnCount(); ++i) {
    std::string name = intable->nameOfColumn(i);
    loadDictionary(name, i, intable);
  }

  // Resize according to meta information
  size_t tableSize = getSize();
  intable->resize(tableSize);
  
  for(size_t i=0; i < intable->columnCount(); ++i) {
    std::string name = intable->nameOfColumn(i);
    loadAttribute(name, i, tableSize, intable);
  }

  return intable;
}

} } // namespace hyrise::io

