// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file Store.h
 *
 * Contains the class definition of Store.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */

#pragma once

#include <map>
#include <set>

#include <storage/MutableVerticalTable.h>
#include <storage/AbstractTable.h>
#include <storage/TableMerger.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/SequentialHeapMerger.h>
#include <storage/PrettyPrinter.h>

#include <helper/types.h>
#include "helper/locking.h"

#include <json.h>


#include "tbb/concurrent_vector.h"

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
  explicit Store(const std::string& tableName, atable_ptr_t main_table);
  virtual ~Store();

  atable_ptr_t getMainTable() const;
  void setDelta(atable_ptr_t _delta);
  atable_ptr_t getDeltaTable() const;
  size_t deltaOffset() const;
  void merge();

  /// Replaces the merger used for merging main tables with delta.
  /// @param _merger Pointer to a merger instance.
  void setMerger(TableMerger* _merger);

  /// Resize the current delta size atomically to new size and return
  /// a pair of start and end for the resized delta that can be used
  /// as a write area that is safe to use
  std::pair<size_t, size_t> resizeDelta(size_t num);
  std::pair<size_t, size_t> appendToDelta(size_t num);

  bool isVisibleForTransaction(pos_t pos, tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const;

  /// This method validates a list of positions to check if it is valid
  void validatePositions(pos_list_t& pos, tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const;
  pos_list_t buildValidPositions(tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const;

  /// Copies a new row to the delta table, sets the validity and the
  /// tx id accordingly. May need to resize delta.
  void copyRowToDelta(const c_atable_ptr_t& source, size_t src_row, size_t dst_row, tx::transaction_id_t tid);
  void copyRowToDeltaFromJSONVector(const std::vector<Json::Value>& source, size_t dst_row, tx::transaction_id_t tid);
  void copyRowToDeltaFromStringVector(const std::vector<std::string>& source, size_t dst_row, tx::transaction_id_t tid);

  void commitPositions(const pos_list_t& pos, const tx::transaction_cid_t cid, bool valid);
  void revertPositions(const pos_list_t& pos, bool valid);

  // TID handling
  inline tx::transaction_id_t tid(size_t row) const { return _tidVector[row]; }
  inline void setTid(size_t row, tx::transaction_id_t tid) { _tidVector[row] = tid; }
  tx::TX_CODE checkForConcurrentCommit(const pos_list_t& pos, tx::transaction_id_t tid) const;
  tx::TX_CODE markForDeletion(pos_t pos, tx::transaction_id_t tid);
  tx::TX_CODE unmarkForDeletion(const pos_list_t& pos, tx::transaction_id_t tid);

  /// AbstractTable interface
  const ColumnMetadata& metadataAt(const size_t column_index,
                                   const size_t row_index = 0,
                                   const table_id_t table_id = 0) const override;

  void setDictionaryAt(adict_ptr_t dict, size_t column, size_t row = 0, table_id_t table_id = 0) override;
  const adict_ptr_t& dictionaryAt(size_t column, size_t row = 0, table_id_t table_id = 0) const override;
  const adict_ptr_t& dictionaryByTableId(size_t column, table_id_t table_id) const override;
  ValueId getValueId(size_t column, size_t row) const override;
  void setValueId(size_t column, size_t row, ValueId vid) override;
  size_t size() const override;
  size_t columnCount() const override;
  unsigned partitionCount() const override;
  size_t partitionWidth(size_t slice) const override;
  void print(size_t limit = (size_t) - 1) const override;
  table_id_t subtableCount() const override { return 2; }
  atable_ptr_t copy() const override;
  const attr_vectors_t getAttributeVectors(size_t column) const override;
  void debugStructure(size_t level = 0) const override;
  void persist_scattered(const pos_list_t& elements, bool new_elements = true) const override;
  void addMainIndex(std::shared_ptr<AbstractIndex> index, std::vector<size_t> columns);
  void addDeltaIndex(std::shared_ptr<AbstractIndex> index, std::vector<size_t> columns);
  void addRowToDeltaIndices(pos_t row);
  std::vector<std::vector<size_t>> getIndexedColumns() const;

  virtual void enableLogging();
  virtual void setName(const std::string name);

  tx::transaction_cid_t getBeginCid(size_t row) {
    return _cidBeginVector[row];
  };
  tx::transaction_cid_t getEndCid(size_t row) {
    return _cidEndVector[row];
  };
  void setBeginCid(size_t row, tx::transaction_cid_t cid) {
    _cidBeginVector[row] = cid;
  };
  void setEndCid(size_t row, tx::transaction_cid_t cid) {
    _cidEndVector[row] = cid;
  };



  tbb::concurrent_vector<tx::transaction_cid_t>::iterator cidBeginIteratorForRecovery() {
    return _cidBeginVector.begin();
  }
  tbb::concurrent_vector<tx::transaction_cid_t>::iterator cidEndIteratorForRecovery() { return _cidEndVector.begin(); }


  size_t checkpointSize() {
    return _checkpoint_size;
  };
  void prepareCheckpoint() {
    _checkpoint_size = size();
    _main_table->prepareCheckpoint();
    delta->prepareCheckpoint();
  }

 private:
  std::atomic<std::size_t> _delta_size;
  // size_t _max_delta_size;

  //* Vector containing the main tables
  atable_ptr_t _main_table;

  //* Current merger
  TableMerger* merger;

  //* Delta store
  atable_ptr_t delta;

  //* checkpointing housekeeping
  size_t _checkpoint_size;

  //* Indices for the Store
  std::vector<std::pair<std::shared_ptr<AbstractIndex>, std::vector<field_t>>> _main_indices, _delta_indices;
  locking::Spinlock _index_lock;

  typedef struct {
    const atable_ptr_t& table;
    size_t offset_in_table;
    size_t table_index;
  } table_offset_idx_t;
  table_offset_idx_t responsibleTable(size_t row) const;

  // TX Management
  // _cidBeginVector stores the CID of the transaction that created the row
  // _cidEndVector stores the CID of the transaction that deleted the row
  tbb::concurrent_vector<tx::transaction_id_t> _cidBeginVector;
  tbb::concurrent_vector<tx::transaction_id_t> _cidEndVector;

  // Stores the TID for each record to identify your own writes
  tbb::concurrent_vector<tx::transaction_id_t> _tidVector;

  friend class PrettyPrinter;
  // main table pos_list
  pos_list_t _main_pos_list;
  // flag if main was updated
  bool _main_dirty;
};
}
}
