// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file Store.h
 *
 * Contains the class definition of Store.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */

#ifndef SRC_LIB_STORAGE_STORE_H_
#define SRC_LIB_STORAGE_STORE_H_

#include <storage/MutableVerticalTable.h>
#include <storage/AbstractTable.h>
#include <storage/TableMerger.h>
#include <storage/LogarithmicMergeStrategy.h>
#include <storage/SequentialHeapMerger.h>
#include <storage/AbstractAllocatedTable.h>

#include <helper/types.h>

enum {
  MainStore,
  DeltaStore
};


/**
 * Store consists of one or more main tables and a delta store and is the
 * only entity capable of modifying the content of the table(s) after
 * initialization via the delta store. It can be merged into the main
 * tables using a to-be-set merger.
 */
class Store : public AbstractTable {
protected:
  //* Vector containing the main tables
  std::vector< hyrise::storage::atable_ptr_t > main_tables;

  //* Delta store
  hyrise::storage::atable_ptr_t delta;

  //* Current merger
  TableMerger *merger;

  typedef struct { const hyrise::storage::atable_ptr_t& table; size_t offset_in_table; size_t table_index; } table_offset_idx_t;
  table_offset_idx_t responsibleTable(size_t row) const;

public:

  explicit Store(std::vector<std::vector<const ColumnMetadata *> *> md);

  explicit Store(hyrise::storage::atable_ptr_t main_table);

  Store();

  virtual ~Store();

  /**
   * Returns a pointer to the main tables vector.
   */
  std::vector< hyrise::storage::atable_ptr_t > getMainTables() const;

  /**
   * Returns a pointer to the delta store.
   */
  hyrise::storage::atable_ptr_t getDeltaTable() const;

  const ColumnMetadata *metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const;

  virtual void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0);

  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const;

  const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const;

  ValueId getValueId(const size_t column, const size_t row) const;
  
  virtual void setValueId(const size_t column, const size_t row, ValueId vid);

  size_t size() const;

  size_t columnCount() const;

  unsigned sliceCount() const;

  virtual void *atSlice(const size_t slice, const size_t row) const;

  virtual size_t getSliceWidth(const size_t slice) const;

  virtual size_t getSliceForColumn(const size_t column) const;
  virtual size_t getOffsetInSlice(const size_t column) const;



  /**
   * Merges main tables with and resets delta.
   * @note Merger must be set!
   */
  void merge();

  void print(const size_t limit = (size_t) - 1) const;

  /**
   * Sets the merger used for merging main tables with delta.
   *
   * @param _merger Pointer to a merger instance.
   */
  void setMerger(TableMerger *_merger);

  /**
   * Sets the delta table.
   *
   * @param _delta New delta.
   */
  void setDelta(hyrise::storage::atable_ptr_t _delta);

  virtual table_id_t subtableCount() const {
    return main_tables.size() + 1;
  }

  virtual  hyrise::storage::atable_ptr_t copy() const;

  virtual const attr_vectors_t getAttributeVectors(size_t column) const;

  virtual void debugStructure(size_t level=0) const;
};

#endif  // SRC_LIB_STORAGE_STORE_H_

