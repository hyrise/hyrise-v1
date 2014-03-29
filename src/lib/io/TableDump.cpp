// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TableDump.h"

#include <errno.h>
#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>

#include <boost/lexical_cast.hpp>

#include "io/LoaderException.h"
#include "io/GenericCSV.h"
#include "io/CSVLoader.h"
#include "io/StorageManager.h"

#include "helper/checked_cast.h"
#include "helper/stringhelpers.h"
#include "helper/vector_helpers.h"
#include "helper/dir.h"

#include "storage/AbstractTable.h"
#include "storage/Store.h"
#include "storage/storage_types.h"
#include "storage/storage_types_helper.h"
#include "storage/meta_storage.h"
#include "storage/GroupkeyIndex.h"
#include "storage/DeltaIndex.h"
#include "storage/ConcurrentUnorderedDictionary.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

#define DUMP_ACTUAL_INDICES

namespace hyrise {
namespace storage {

namespace DumpHelper {
static const std::string META_DATA_EXT = "metadata.dat";
static const std::string HEADER_EXT = "header.dat";
static const std::string DICT_EXT = ".dict.dat";
static const std::string ATTR_EXT = ".attr.dat";
static const std::string INDEX_EXT = "indices.dat";

static inline std::string buildPath(std::initializer_list<std::string> l) {
  return functional::foldLeft(l, std::string(), infix("/"));
}
}

struct CreateGroupkeyIndexFunctor {
  typedef aindex_ptr_t value_type;
  const c_atable_ptr_t& in;
  size_t column;
  std::string& name;

  CreateGroupkeyIndexFunctor(const c_atable_ptr_t& t, size_t c, std::string& n) : in(t), column(c), name(n) {}

  template <typename R>
  value_type operator()() {
    return std::make_shared<GroupkeyIndex<R>>(in, column, true, name);
  }
};

struct DumpGroupkeyIndexFunctor {
  typedef void value_type;
  aindex_ptr_t& in;
  std::ofstream& out;

  DumpGroupkeyIndexFunctor(aindex_ptr_t& idx, std::ofstream& s) : in(idx), out(s) {}

  template <typename R>
  value_type operator()() {
    auto idx = checked_pointer_cast<GroupkeyIndex<R>>(in);
    cereal::BinaryOutputArchive archive(out);
    archive(*idx.get());
  }
};

struct RestoreGroupkeyIndexFunctor {
  typedef aindex_ptr_t value_type;
  const c_atable_ptr_t& t;
  size_t column;
  std::ifstream& in;
  std::string& name;

  RestoreGroupkeyIndexFunctor(const c_atable_ptr_t& t, size_t c, std::ifstream& s, std::string& n)
      : t(t), column(c), in(s), name(n) {}

  template <typename R>
  value_type operator()() {
    auto idx = std::make_shared<GroupkeyIndex<R>>(t, column, false, name, true);
    cereal::BinaryInputArchive archive(in);
    archive(*idx.get());
    return idx;
  }
};

struct CreateDeltaIndexFunctor {
  typedef aindex_ptr_t value_type;
  std::string& name;
  size_t size;

  CreateDeltaIndexFunctor(std::string& n, size_t s) : name(n), size(s) {}

  template <typename R>
  value_type operator()() {
    return std::make_shared<DeltaIndex<R>>(name, size);
  }
};

/**
 * This functor is used to write directly a typed value to a stream
 */
struct write_to_stream_functor {
  // this functor is only used for main dictionaries, therefore we dont
  // need to take special care of not chasing updates, because the main dict is fix
  // during normal runtime
  typedef void value_type;
  std::ofstream& file;
  adict_ptr_t dict;

  write_to_stream_functor(std::ofstream& o, adict_ptr_t d) : file(o), dict(d) {}

  template <typename R>
  inline void operator()() {
    size_t size = dict->size();
    const auto& typedDict = std::dynamic_pointer_cast<BaseDictionary<R>>(dict);
    file.write((const char*)&size, sizeof(size_t));
    for (auto it = typedDict->begin(); it != typedDict->end(); ++it) {
      file.write((const char*)&(*it), sizeof(R));
    }
  }
};
template <>
void write_to_stream_functor::operator()<hyrise_string_t>() {
  size_t size = dict->size();
  const auto& typedDict = std::dynamic_pointer_cast<BaseDictionary<hyrise_string_t>>(dict);
  file.write((const char*)&size, sizeof(size_t));
  for (auto it = typedDict->begin(); it != typedDict->end(); ++it) {
    size_t s = it->size();
    file.write((const char*)&s, sizeof(size_t));
    file.write(it->c_str(), s);
  }
}



/**
 * This functor is used to write directly a typed value to a stream, from a ConcurrentUnorderedDictionary
 * It accesses the unordered values only, the map on top has to be rebuilt on load.
 */
struct write_to_stream_functor_delta_dict {
  typedef void value_type;
  std::ofstream& file;
  adict_ptr_t dict;

  write_to_stream_functor_delta_dict(std::ofstream& o, adict_ptr_t d) : file(o), dict(d) {}
  template <typename R>
  inline void operator()() {
    const auto& typedDict = checked_pointer_cast<ConcurrentUnorderedDictionary<R>>(dict);
    size_t size = typedDict->checkpointSize();
    file.write((const char*)&size, sizeof(size_t));
    auto it = typedDict->unsorted_begin();
    auto end_iter = it + size;
    for (; it != end_iter; ++it) {
      file.write((const char*)&(*it), sizeof(R));
    }
  }
};
template <>
void write_to_stream_functor_delta_dict::operator()<hyrise_string_t>() {
  const auto& typedDict = checked_pointer_cast<ConcurrentUnorderedDictionary<hyrise_string_t>>(dict);
  size_t size = typedDict->checkpointSize();
  file.write((const char*)&size, sizeof(size_t));
  auto it = typedDict->unsorted_begin();
  auto end_iter = it + size;

  for (; it != end_iter; ++it) {
    size_t s = it->size();
    file.write((const char*)&s, sizeof(size_t));
    file.write(it->c_str(), s);
  }
}


struct write_to_dict_functor_mmap {
  typedef void value_type;
  const char* data;
  atable_ptr_t table;
  field_t col;

  write_to_dict_functor_mmap(const char* ptr, atable_ptr_t t, field_t c) : data(ptr), table(t), col(c) {}

  template <typename R>
  inline void operator()() {
    auto dict = std::dynamic_pointer_cast<BaseDictionary<R>>(table->dictionaryAt(col));
    const R* ptr = (R*)(data + sizeof(size_t));
    size_t size = *((size_t*)data);  // first sizeof(size_t) bytes store dictionary size;
    dict->reserve(size);
    for (size_t i = 0; i < size; ++i) {
      dict->addValue(*(ptr++));
    }
  }
};
template <>
void write_to_dict_functor_mmap::operator()<hyrise_string_t>() {
  auto dict = std::dynamic_pointer_cast<BaseDictionary<hyrise_string_t>>(table->dictionaryAt(col));
  size_t size = *((size_t*)data);  // first sizeof(size_t) bytes store dictionary size;
  dict->reserve(size);
  const size_t* sptr = (size_t*)(data + sizeof(size_t));
  const char* cptr = data + 2 * sizeof(size_t);
  size_t read;
  for (size_t i = 0; i < size; ++i) {
    std::string val(cptr, *sptr);
    dict->addValue(val);
    read = *sptr;
    sptr = (size_t*)(cptr + read);
    cptr = cptr + read + sizeof(size_t);
  }
}



/**
 * Helper functor used to read all values from a file and insert them
 * in the file order directly to the ConcurrentUnorderedDictionary.
 * The Index/Map/BTree of the Dictionary is automatically rebuilt in the
 * addValue() method.
 */

struct write_to_delta_vector_functor {
  typedef void value_type;
  std::ifstream& data;
  atable_ptr_t table;
  field_t col;
  write_to_delta_vector_functor(std::ifstream& d, atable_ptr_t t, field_t c) : data(d), table(t), col(c) {
    // data.rdbuf()->pubsetbuf(iobuf, sizeof iobuf);
  }
  template <typename R>
  inline void operator()() {
    auto dict = checked_pointer_cast<ConcurrentUnorderedDictionary<R>>(table->dictionaryAt(col));
    size_t size;
    data.read((char*)&size, sizeof(size_t));
    std::vector<R> values(size);
    data.read((char*)&values[0], size * sizeof(R));
    for (const auto value : values) {
      dict->addValue(value);
    }
  }
};
template <>
void write_to_delta_vector_functor::operator()<hyrise_string_t>() {
  auto dict = checked_pointer_cast<ConcurrentUnorderedDictionary<hyrise_string_t>>(table->dictionaryAt(col));
  size_t size;
  // copy whole file to buffer first
  data.seekg(0, data.end);
  int length = data.tellg();
  data.seekg(0, data.beg);
  char* buffer = new char[length];
  data.read(buffer, length);
  char* position_in_buffer = buffer;
  // file's format is (int)nr_of_entries, [(int)length_of_string, string]
  memcpy(&size, position_in_buffer, sizeof(size_t));
  position_in_buffer += sizeof(size_t);

  for (size_t i = 0; i < size; ++i) {
    size_t s;
    memcpy(&s, position_in_buffer, sizeof(size_t));
    position_in_buffer += sizeof(size_t);
    std::string tmp(s, '\0');
    memcpy(&tmp[0], position_in_buffer, s);
    position_in_buffer += s;
    dict->addValue(tmp);
  }
  delete[] buffer;
  if (position_in_buffer != (buffer + length)) {
    throw std::runtime_error("Warning, did not read whole file.");
  }
  // Equivalent on regular file object, without buffer:
  // for (size_t i=0; i<size; ++i) {
  //   size_t s;
  //   data.read((char*) &s, sizeof(size_t));
  //   std::string tmp(s, '\0');
  //   data.read(&tmp[0], s);
  //   dict->addValue(tmp);
  // }
}

void SimpleTableDump::prepare(std::string name) {
  std::string fullPath = _baseDirectory + "/" + name;
  _mkdir(fullPath);
}

void SimpleTableDump::dumpDictionary(std::string name, atable_ptr_t table, size_t col, bool delta) {
  std::string fullPath = _baseDirectory + "/" + name + "/" + table->nameOfColumn(col) + ".dict.dat";
  std::ofstream data(fullPath, std::ios::out | std::ios::binary);
  if (!delta) {
    // We make a small hack here, first we obtain the size of the
    // dictionary then we virtually create all value ids, this can break
    // if the dictionary has no contigous value ids
    // size_t dictionarySize = table->dictionaryAt(col)->size();
    write_to_stream_functor fun(data, table->dictionaryAt(col));  // will pick main dictionary by default for stores
    type_switch<hyrise_basic_types> ts;
    ts(table->typeOfColumn(col), fun);
    /*for(size_t i=0; i < dictionarySize; ++i) {
      fun.setCol(col);
      fun.setVid(i);
      ts(table->typeOfColumn(col), fun);
    }*/
  } else {
    write_to_stream_functor_delta_dict fun(
        data, table->dictionaryAt(col));  // will pick main dictionary by default for stores
    type_switch<hyrise_basic_types> ts;
    ts(table->typeOfColumn(col), fun);
  }
  data.close();
}

void SimpleTableDump::dumpAttribute(std::string name, atable_ptr_t table, size_t col) {
  assert(std::dynamic_pointer_cast<Store>(table) ==
         nullptr);  // this should never be called with a store directly, but with main and delta table sepratly.
  std::string fullPath = _baseDirectory + "/" + name + "/" + table->nameOfColumn(col) + ".attr.dat";
  std::ofstream data(fullPath, std::ios::out | std::ios::binary);

  // size_t tableSize = table->size(); // get size before, to avoid chasing updates..
  auto tableSize = table->checkpointSize();

  std::vector<value_id_t> vidVector;
  vidVector.resize(tableSize);

  for (size_t i = 0; i < tableSize; ++i) {
    ValueId v;
    v = table->getValueId(col, i);
    vidVector[i] = v.valueId;
  }
  data.write((char*)&vidVector[0], tableSize * sizeof(value_id_t));
  data.close();
}

void SimpleTableDump::dumpHeader(std::string name, atable_ptr_t table) {
  std::stringstream header;
  std::vector<std::string> names, types;
  std::vector<uint32_t> parts;

  // Get names and types
  for (size_t i = 0; i < table->columnCount(); ++i) {
    names.push_back(table->nameOfColumn(i));
    types.push_back(data_type_to_string(table->typeOfColumn(i)));
  }

  // This calculation will break if the width of the value_id changes
  // or someone forgets to simply update the width accordingly in the
  // constructor of the table
  for (size_t i = 0; i < table->partitionCount(); ++i) {
    parts.push_back(table->partitionWidth(i));
  }

  // Dump and join
  header << std::accumulate(names.begin(), names.end(), std::string(), infix(" | ")) << "\n";
  header << std::accumulate(types.begin(), types.end(), std::string(), infix(" | ")) << "\n";
  std::vector<std::string> allParts;
  for (size_t i = 0; i < parts.size(); ++i) {
    auto p = parts[i];
    auto tmp = std::vector<std::string>(p, std::to_string(i) + "_R");
    allParts.insert(allParts.end(), tmp.begin(), tmp.end());
  }
  header << std::accumulate(allParts.begin(), allParts.end(), std::string(), infix(" | ")) << "\n";
  header << "===";

  std::string fullPath = _baseDirectory + "/" + name + "/header.dat";
  std::ofstream data(fullPath, std::ios::out | std::ios::binary);
  data << header.str();
  data.close();
}

void SimpleTableDump::dumpMetaData(std::string name, atable_ptr_t table) {
  std::string fullPath = _baseDirectory + "/" + name + "/metadata.dat";
  std::ofstream data(fullPath, std::ios::out | std::ios::binary);
  data << table->checkpointSize();
  data.close();
}

void SimpleTableDump::dumpIndices(std::string name, store_ptr_t store) {
  auto indexedColumns = store->getIndexedColumns();
  if (!indexedColumns.empty()) {
    std::string fullPath = _baseDirectory + "/" + name + "/indices.dat";
    std::ofstream data(fullPath, std::ios::trunc | std::ios::binary);
    std::ostream_iterator<size_t> it(data);
    size_t total = indexedColumns.size();
    data.write((char*)&total, sizeof(size_t));
    for (const auto& compounds : indexedColumns) {
      size_t s = compounds.size();
      data.write((char*)&s, sizeof(size_t));
      for (size_t col : compounds) {
        data.write((char*)&col, sizeof(size_t));
      }
    }
    data.close();

#ifdef DUMP_ACTUAL_INDICES
    for (const auto idxCols : indexedColumns) {
      std::string idxName = "mcidx__" + name + "__main";
      for (auto col : idxCols) {
        idxName += "__" + store->nameOfColumn(col);
      }
      auto idxMain = io::StorageManager::getInstance()->getInvertedIndex(idxName);

      // dump main index, expected to be of type GroupkeyIndex
      fullPath = _baseDirectory + "/" + name + "/" + idxName + ".dat";
      std::ofstream outstream(fullPath, std::ios::trunc | std::ios::binary);

      if (idxCols.size() == 1) {
        DumpGroupkeyIndexFunctor fun(idxMain, outstream);
        type_switch<hyrise_basic_types> ts;
        ts(store->typeOfColumn(idxCols[0]), fun);
      } else {
        auto idx = checked_pointer_cast<GroupkeyIndex<compound_valueid_key_t>>(idxMain);
        cereal::BinaryOutputArchive archive(outstream);
        archive(*idx.get());
      }
      outstream.close();
    }
#else

#endif
  }
}

void SimpleTableDump::verify(atable_ptr_t table) {
  auto res = std::dynamic_pointer_cast<Store>(table);
  if (!res)
    throw std::runtime_error("Can only dump Stores");

  if (res->subtableCount() <= 1)
    throw std::runtime_error("Store must have at least one main table");
  if (res->subtableCount() != 2)
    throw std::runtime_error("Multi-generation stores are not supported for dumping");
}

bool SimpleTableDump::dump(std::string name, atable_ptr_t table) {
  verify(table);
  auto mainTable = std::dynamic_pointer_cast<Store>(table)->getMainTable();
  prepare(name);
  for (size_t i = 0; i < mainTable->columnCount(); ++i) {
    // For each attribute dump dictionary and values
    dumpDictionary(name, mainTable, i);
    dumpAttribute(name, mainTable, i);
  }

  dumpHeader(name, mainTable);
  dumpMetaData(name, mainTable);
  dumpIndices(name, std::dynamic_pointer_cast<Store>(table));

  return true;
}

bool SimpleTableDump::dumpDelta(std::string name, atable_ptr_t table) {
  verify(table);
  auto deltaTable = std::dynamic_pointer_cast<Store>(table)->getDeltaTable();
  prepare(name);
  for (size_t i = 0; i < deltaTable->columnCount(); ++i) {
    // For each attribute dump dictionary and values
    dumpDictionary(name, deltaTable, i, true);
    dumpAttribute(name, deltaTable, i);
  }

  dumpHeader(name, deltaTable);
  dumpMetaData(name, deltaTable);

  return true;
}

bool SimpleTableDump::dumpCidVectors(std::string name, atable_ptr_t table) {
  verify(table);
  prepare(name);
  auto store = std::dynamic_pointer_cast<Store>(table);

  std::string fullPath_begin = _baseDirectory + "/" + name + "/begin.cid.dat";
  std::string fullPath_end = _baseDirectory + "/" + name + "/end.cid.dat";
  std::ofstream data_begin(fullPath_begin, std::ios::out | std::ios::binary);
  std::ofstream data_end(fullPath_end, std::ios::out | std::ios::binary);

  // note: The cid vectors are different for NV built and normal, so we better copy it once, instead of accessing the
  // memory underneath directly.
  std::vector<tx::transaction_cid_t> beginCid;
  std::vector<tx::transaction_cid_t> endCid;
  size_t store_size = store->checkpointSize();

  // copy everything to our std::vectors & write out to file
  beginCid.resize(store_size);
  auto cid_start_begin = store->cidBeginIteratorForRecovery();
  std::copy(cid_start_begin, cid_start_begin + store_size, beginCid.begin());
  data_begin.write((char*)&beginCid[0], store_size * sizeof(tx::transaction_cid_t));
  data_begin.close();

  endCid.resize(store_size);
  auto cid_end_begin = store->cidEndIteratorForRecovery();
  std::copy(cid_end_begin, cid_end_begin + store_size, endCid.begin());
  data_end.write((char*)&endCid[0], store_size * sizeof(tx::transaction_cid_t));
  data_end.close();

  return true;
}

}  // namespace storage

namespace io {

size_t TableDumpLoader::getSize() {
  std::string path = storage::DumpHelper::buildPath({_base, _table, storage::DumpHelper::META_DATA_EXT});
  std::ifstream data(path, std::ios::binary);
  size_t numRows;
  data >> numRows;
  data.close();
  return numRows;
}

void TableDumpLoader::loadDictionary(std::string name, size_t col, storage::atable_ptr_t intable) {
  std::string path = storage::DumpHelper::buildPath({_base, _table, name}) + storage::DumpHelper::DICT_EXT;

  // determine file size
  struct stat stbuf;
  if (stat(path.c_str(), &stbuf) < 0) {
    throw std::runtime_error("unable to determine file size");
  }
  const size_t fsize = stbuf.st_size;

  // memory map the file
  int fd = open(path.c_str(), O_RDONLY);
  void* ptr = (char*)mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
  storage::write_to_dict_functor_mmap fun((const char*)ptr, intable, col);
  storage::type_switch<hyrise_basic_types> ts;
  ts(intable->typeOfColumn(col), fun);
  munmap(ptr, fsize);
  close(fd);
}

void TableDumpLoader::loadDeltaDictionary(std::string name, size_t col, storage::atable_ptr_t intable) {
  std::string path = storage::DumpHelper::buildPath({_base, _table, name}) + storage::DumpHelper::DICT_EXT;
  std::ifstream data(path, std::ios::binary);

  storage::write_to_delta_vector_functor fun(data, intable, col);
  storage::type_switch<hyrise_basic_types> ts;
  ts(intable->typeOfColumn(col), fun);

  data.close();
}


void TableDumpLoader::loadAttribute(std::string name, size_t col, size_t size, storage::atable_ptr_t intable) {
  std::string path = storage::DumpHelper::buildPath({_base, _table, name}) + storage::DumpHelper::ATTR_EXT;
  std::ifstream data(path, std::ios::binary);

  std::vector<value_id_t> inputVector;
  inputVector.resize(size);

  data.read((char*)&inputVector[0], size * sizeof(value_id_t));

  for (size_t i = 0; i < size; ++i) {
    ValueId vid;
    vid.valueId = inputVector[i];
    intable->setValueId(col, i, vid);
  }

  data.close();
}

void TableDumpLoader::loadIndices(storage::atable_ptr_t intable) {
  std::string path = storage::DumpHelper::buildPath({_base, _table + "/"}) + storage::DumpHelper::INDEX_EXT;
  std::ifstream data(path, std::ios::binary);
  if (data) {
    std::vector<std::vector<size_t>> indexedColumns;
    size_t total;
    data.read((char*)&total, sizeof(size_t));
    for (size_t i = 0; i < total; ++i) {
      size_t n;
      data.read((char*)&n, sizeof(size_t));
      std::vector<size_t> tmp(n);
      for (size_t j = 0; j < n; ++j) {
        data.read((char*)&tmp[j], sizeof(size_t));
      }
      indexedColumns.push_back(tmp);
    }
    data.close();

    for (auto& idxCols : indexedColumns) {
      std::string idxName = "mcidx__" + _table + "__main";
      std::string idxDeltaName = "mcidx__" + _table + "__delta";
      for (auto col : idxCols) {
        idxName += "__" + intable->nameOfColumn(col);
        idxDeltaName += "__" + intable->nameOfColumn(col);
      }
#ifdef DUMP_ACTUAL_INDICES
      std::string idxPath = _base + "/" + _table + "/" + idxName + ".dat";
      std::ifstream instream(idxPath, std::ios::binary);
      storage::RestoreGroupkeyIndexFunctor funMain(intable, idxCols[0], instream, idxName);
#else
      storage::CreateGroupkeyIndexFunctor funMain(intable, idxCols[0], idxName);
#endif

      auto store = std::dynamic_pointer_cast<storage::Store>(intable);

      storage::aindex_ptr_t idxMain, idxDelta;
      if (idxCols.size() == 1) {
        size_t column = idxCols[0];
        storage::type_switch<hyrise_basic_types> tsMain;
        idxMain = tsMain(intable->typeOfColumn(column), funMain);

        storage::CreateDeltaIndexFunctor funDelta(idxDeltaName, 10000);  // TODO: magic constant
        storage::type_switch<hyrise_basic_types> tsDelta;
        idxDelta = tsDelta(intable->typeOfColumn(column), funDelta);
      } else {
        auto idx =
            std::make_shared<storage::GroupkeyIndex<compound_valueid_key_t>>(intable, idxCols, false, idxName, true);
        cereal::BinaryInputArchive archive(instream);
        archive(*idx.get());
        idxMain = idx;

        idxDelta = std::make_shared<storage::DeltaIndex<compound_value_key_t>>(idxDeltaName, 100000);  // TODO: magic
      }

      StorageManager::getInstance()->addInvertedIndex(idxName, idxMain);
      StorageManager::getInstance()->addInvertedIndex(idxDeltaName, idxDelta);
      store->addDeltaIndex(idxDelta, idxCols);
    }
  }
}

std::shared_ptr<storage::AbstractTable> TableDumpLoader::load(storage::atable_ptr_t intable,
                                                              const storage::compound_metadata_list* meta,
                                                              const Loader::params& args) {
  // Resize according to meta information
  size_t tableSize = getSize();
  intable->resize(tableSize);

  if (args.getDeltaDataStructure() == false) {
    for (size_t i = 0; i < intable->columnCount(); ++i) {
      std::string name = intable->nameOfColumn(i);
      loadDictionary(name, i, intable);
      loadAttribute(name, i, tableSize, intable);
    }
  } else {
    for (size_t i = 0; i < intable->columnCount(); ++i) {
      std::string name = intable->nameOfColumn(i);
      loadDeltaDictionary(name, i, intable);
      loadAttribute(name, i, tableSize, intable);
    }
  }
  return intable;
}

bool TableDumpLoader::loadCidVectors(std::string name, storage::atable_ptr_t table) {
  auto store = std::dynamic_pointer_cast<storage::Store>(table);

  std::string fullPath_begin = _base + "/" + name + "/begin.cid.dat";
  std::string fullPath_end = _base + "/" + name + "/end.cid.dat";

  std::ifstream data_begin(fullPath_begin, std::ios::binary);
  std::ifstream data_end(fullPath_end, std::ios::binary);

  std::vector<tx::transaction_cid_t> beginCid;
  std::vector<tx::transaction_cid_t> endCid;
  // read complete vector and set ids.
  size_t store_size = store->size();
  beginCid.resize(store_size);
  endCid.resize(store_size);
  data_begin.read((char*)&beginCid[0], store_size * sizeof(tx::transaction_cid_t));


  data_end.read((char*)&endCid[0], store_size * sizeof(tx::transaction_cid_t));

  std::copy(beginCid.begin(), beginCid.end(), store->cidBeginIteratorForRecovery());
  std::copy(endCid.begin(), endCid.end(), store->cidEndIteratorForRecovery());

  data_begin.close();
  data_end.close();


  return true;
}
}
}  // namespace hyrise::io
