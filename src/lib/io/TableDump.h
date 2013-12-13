// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "io/AbstractLoader.h"


namespace hyrise { namespace storage {

class AbstractTable;
/**
 * This is the class that allows dumping a table instance in a very
 * simple way directly to the file system without using a third party
 * library or anything else.
 *
 * The idea behind this method is that, for a given table a directory
 * is created and inside the directory all information is stored. This
 * means that for all attributes a single file is created that
 * persists all valueIds of the attribute. In addition for each
 * dictionary a file is created that stores all dictionary values in a
 * sorted order. As a last part the metadata of the table is stored in
 * two files. The first file contains the layout for the table and the
 * second table contains the size information for the table.
 */
class SimpleTableDump {
  std::string _baseDirectory;

  /**
   * Create all required directories;
   */
  void prepare(std::string name);

  /**
   * Dumps the dictionary and performs simple conversion based on the
   * ofstream structure
   */
  void dumpDictionary(std::string name, std::shared_ptr<AbstractTable> t, size_t col);

  /**
   * Dumps the attrbite but writes the attribute data binary since its
   * all value_id_t
   */
  void dumpAttribute(std::string name, std::shared_ptr<AbstractTable> t, size_t col);

  /**
   */
  void dumpMetaData(std::string name, std::shared_ptr<AbstractTable> t);

  /**
   */
  void dumpHeader(std::string name, std::shared_ptr<AbstractTable> t);

  /**
   * Check if the file is a store and not a horizontal table
   */
  void verify(std::shared_ptr<AbstractTable>);

public:

  // Initialize a new object based on the base path for the output
  explicit SimpleTableDump(std::string outputDir): _baseDirectory(outputDir) {
  }

  /**
   * For a table identified by name and table perform the dump
   */
  bool dump(std::string name, std::shared_ptr<AbstractTable> table);
};

} // namespace storage

namespace io {

class TableDumpLoader : public AbstractInput {
  std::string _base;
  std::string _table;

  size_t getSize();

  void loadDictionary(std::string name, size_t col, std::shared_ptr<storage::AbstractTable> intable);

  void loadAttribute(std::string name,
                     size_t col,
                     size_t size,
                     std::shared_ptr<storage::AbstractTable> intable);

public:
  TableDumpLoader(std::string base, std::string table) :
    _base(base), _table(table) {
  }

  std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable>,
                                               const storage::compound_metadata_list *,
                                               const Loader::params &args);

  bool needs_store_wrap() {
    return true;
  }

  TableDumpLoader *clone() const {
    return new TableDumpLoader(*this);
  }
};

} } // namespace hyrise::io

