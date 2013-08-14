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
#include <storage/AbstractMergeStrategy.h>
#include <storage/SequentialHeapMerger.h>

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

  // TX Management
  // Stores a flag per row indicating if it is valid or not
  std::vector<bool> _validityVector;
  // Stores the CID for each row when it was modified or UNKNOWN if it was not changed
  std::vector<hyrise::tx::transaction_id_t> _cidVector;
  // Stores the TID for each record to read your own writes
  std::vector<hyrise::tx::transaction_id_t> _tidVector;

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

  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0) const;

  const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const;

  ValueId getValueId(const size_t column, const size_t row) const;
  
  virtual void setValueId(const size_t column, const size_t row, ValueId vid);

  size_t size() const;

  size_t deltaOffset() const;

  size_t columnCount() const;

  unsigned partitionCount() const;

  virtual size_t partitionWidth(const size_t slice) const;

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

  void setDefaultMerger();

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

  /**
  * Resize the current delta size atomically to new size and return a pair of
  * start and end for the resized delta that can be used as a write area that
  * is safe to use
  */
  std::pair<size_t, size_t> resizeDelta(size_t num);

  std::pair<size_t, size_t> appendToDelta(size_t num);

  /**
  * This method validates a list of positions to check if it is valid
  */
  void validatePositions(pos_list_t& pos, hyrise::tx::transaction_cid_t last_commit_id, hyrise::tx::transaction_id_t tid ) const;

  pos_list_t buildValidPositions(hyrise::tx::transaction_cid_t last_commit_id, hyrise::tx::transaction_id_t tid, bool& all ) const;

  /*
  * Copies a new row to the delta table, sets the validity and the tx id
  * accordningly. It has to be made sure that the delta is resized accordingly
  */
  void copyRowToDelta(const hyrise::storage::c_atable_ptr_t& source, const size_t src_row, const size_t dst_row, hyrise::tx::transaction_id_t tid, bool valid);

  hyrise::tx::TX_CODE checkCommitID(const pos_list_t& pos, hyrise::tx::transaction_cid_t old_cid);

  hyrise::tx::TX_CODE updateCommitID(const pos_list_t& pos, hyrise::tx::transaction_cid_t cid, bool valid );

  inline bool valid(size_t row) const { return _validityVector[row]; }
  
  // TID handling
  inline hyrise::tx::transaction_id_t tid(size_t row) const { return _tidVector[row]; }

  inline void setTid(size_t row, hyrise::tx::transaction_id_t tid) { _tidVector[row] = tid; }

  inline bool cid(size_t row) const { return _cidVector[row]; }  

};

#endif  // SRC_LIB_STORAGE_STORE_H_

