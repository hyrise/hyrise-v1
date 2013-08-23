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
#include <storage/PrettyPrinter.h>

#include <helper/types.h>

namespace hyrise {
namespace storage {

/**
 * Store consists of one or more main tables and a delta store and is the
 * only entity capable of modifying the content of the table(s) after
 * initialization via the delta store. It can be merged into the main
 * tables using a to-be-set merger.
 */
class Store : public AbstractTable {
public:
  Store();
  explicit Store(atable_ptr_t main_table);
  virtual ~Store();

  std::vector< atable_ptr_t > getMainTables() const;
  void setDelta(atable_ptr_t _delta);
  atable_ptr_t getDeltaTable() const;
  size_t deltaOffset() const;
  void merge();

  /// Replaces the merger used for merging main tables with delta.
  /// @param _merger Pointer to a merger instance.
  void setMerger(TableMerger *_merger);

  /// Resize the current delta size atomically to new size and return
  /// a pair of start and end for the resized delta that can be used
  /// as a write area that is safe to use
  std::pair<size_t, size_t> resizeDelta(size_t num);
  std::pair<size_t, size_t> appendToDelta(size_t num);

  bool isVisibleForTransaction(pos_t pos, tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const;

  /// This method validates a list of positions to check if it is valid
  void validatePositions(pos_list_t& pos, tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid ) const;
  pos_list_t buildValidPositions(tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const;

  /// Copies a new row to the delta table, sets the validity and the
  /// tx id accordingly. May need to resize delta.
  void copyRowToDelta(const c_atable_ptr_t& source, size_t src_row, size_t dst_row, tx::transaction_id_t tid);

  tx::TX_CODE commitPositions(const pos_list_t& pos, const tx::transaction_cid_t cid, bool valid);

  // TID handling
  inline tx::transaction_id_t tid(size_t row) const { return _tidVector[row]; }
  inline void setTid(size_t row, tx::transaction_id_t tid) { _tidVector[row] = tid; }
  tx::TX_CODE checkForConcurrentCommit(const pos_list_t& pos, tx::transaction_id_t tid) const;
  tx::TX_CODE markForDeletion(pos_t pos,  tx::transaction_id_t tid);
  tx::TX_CODE unmarkForDeletion(const pos_list_t& pos, tx::transaction_id_t tid);

  /// AbstractTable interface
  const ColumnMetadata *metadataAt(size_t column_index, size_t row_index = 0, table_id_t table_id = 0) const override;
  void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, size_t column, size_t row = 0, table_id_t table_id = 0) override;
  const AbstractTable::SharedDictionaryPtr& dictionaryAt(size_t column, size_t row = 0, table_id_t table_id = 0) const override;
  const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(size_t column, table_id_t table_id) const override;
  ValueId getValueId(size_t column, size_t row) const override;
  void setValueId(size_t column, size_t row, ValueId vid) override;
  size_t size() const override;
  size_t columnCount() const override;
  unsigned partitionCount() const override;
  size_t partitionWidth(size_t slice) const override;
  void print(size_t limit = (size_t) - 1) const override;
  table_id_t subtableCount() const override {
    return main_tables.size() + 1;
  }
  atable_ptr_t copy() const override;
  const attr_vectors_t getAttributeVectors(size_t column) const override;
  void debugStructure(size_t level=0) const override;

 private:
  //* Vector containing the main tables
  std::vector< atable_ptr_t > main_tables;

  //* Delta store
  atable_ptr_t delta;

  //* Current merger
  TableMerger *merger;

  typedef struct { const atable_ptr_t& table; size_t offset_in_table; size_t table_index; } table_offset_idx_t;
  table_offset_idx_t responsibleTable(size_t row) const;

  // TX Management
  // Stores the CID of the transaction that created the row
  std::vector<tx::transaction_id_t> _cidBeginVector;
  // Stores the CID of the transaction that deleted the row
  std::vector<tx::transaction_id_t> _cidEndVector;
  // Stores the TID for each record to identify your own writes
  std::vector<tx::transaction_id_t> _tidVector;
  friend class PrettyPrinter;
};

}}


#endif  // SRC_LIB_STORAGE_STORE_H_
