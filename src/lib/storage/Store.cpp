// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/Store.h>
#include <iostream>

#include "storage/PrettyPrinter.h"
#include "io/TransactionManager.h"

#include <helper/vector_helpers.h>
#include <helper/locking.h>
#include <helper/cas.h>

Store::Store(std::vector<std::vector<const ColumnMetadata *> *> md) :
  merger(nullptr) {
  throw std::runtime_error("Bad things happende");
}

Store::Store() :
  merger(nullptr) {
  setDefaultMerger();
  setUuid();

}

Store::Store(hyrise::storage::atable_ptr_t main_table) :
  delta(main_table->copy_structure_modifiable()),
  merger(nullptr),
  _cidBeginVector(main_table->size(), 0),
  _cidEndVector(main_table->size(), hyrise::tx::INF_CID),
  _tidVector(main_table->size(), hyrise::tx::UNKNOWN) {

  setUuid();
  main_tables.push_back(main_table);
  setDefaultMerger();
}

Store::~Store() {
  delete merger;
}

void Store::merge() {
  if (merger == nullptr) {
    throw std::runtime_error("No Merger set.");
  }

  // Create new delta and merge
  hyrise::storage::atable_ptr_t new_delta = delta->copy_structure_modifiable();

  // Prepare the merge
  std::vector<hyrise::storage::c_atable_ptr_t> tmp(main_tables.begin(), main_tables.end());
  tmp.push_back(delta);

  // get valid positions
  std::vector<bool> validPositions(_cidBeginVector.size());
  hyrise::functional::forEachWithIndex(_cidBeginVector, [&](size_t i, bool v){
    validPositions[i] = isVisibleForTransaction(i, hyrise::tx::TransactionManager::getInstance().getLastCommitId(), hyrise::tx::MERGE_TID);
  });

  main_tables = merger->merge(tmp, true, validPositions);

  // Fixup the cid and tid vectors
  _cidBeginVector = std::vector<hyrise::tx::transaction_cid_t>(main_tables[0]->size(), hyrise::tx::UNKNOWN_CID);
  _cidEndVector = std::vector<hyrise::tx::transaction_cid_t>(main_tables[0]->size(), hyrise::tx::INF_CID);
  _tidVector = std::vector<hyrise::tx::transaction_id_t>(main_tables[0]->size(), hyrise::tx::UNKNOWN);

  // Replace the delta partition
  delta = new_delta;
}


std::vector< hyrise::storage::atable_ptr_t > Store::getMainTables() const {
  return main_tables;
}

hyrise::storage::atable_ptr_t Store::getDeltaTable() const {
  return delta;
}

const ColumnMetadata *Store::metadataAt(const size_t column_index, const size_t row_index, const table_id_t table_id) const {
  size_t offset = 0;

  for (size_t main = 0; main < main_tables.size(); main++)
    if (main_tables[main]->size() + offset > row_index) {
      return main_tables[main]->metadataAt(column_index, row_index - offset, table_id);
    } else {
      offset += main_tables[main]->size();
    }

  // row is not in main tables. return metadata from delta
  return delta->metadataAt(column_index, row_index - offset, table_id);
}

void Store::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row, const table_id_t table_id) {
  size_t offset = 0;

  for (size_t main = 0; main < main_tables.size(); main++)
    if (main_tables[main]->size() + offset > row) {
      main_tables[main]->setDictionaryAt(dict, column, row - offset, table_id);
      return;
    } else {
      offset += main_tables[main]->size();
    }

  // row is not in main tables. set dict in delta
  delta->setDictionaryAt(dict, column, row - offset, table_id);
}

const AbstractTable::SharedDictionaryPtr& Store::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id) const {
  // if (!row) {
  //   return this->dictionaryByTableId(column, table_id);
  // }

  // if (table_id) {
  //   if (table_id < main_tables.size()) {
  //     return main_tables[table_id]->dictionaryAt(column, row);
  //   } else {
  //     return delta->dictionaryAt(column, row);
  //   }
  // }

  size_t offset = 0;

  for (size_t main = 0; main < main_tables.size(); main++)
    if (main_tables[main]->size() + offset > row) {
      return main_tables[main]->dictionaryAt(column, row - offset);
    } else {
      offset += main_tables[main]->size();
    }

  // row is not in main tables. return row from delta
  return delta->dictionaryAt(column, row - offset);
}

const AbstractTable::SharedDictionaryPtr& Store::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  assert(table_id <= main_tables.size());

  if (table_id < main_tables.size()) {
    return main_tables[table_id]->dictionaryByTableId(column, table_id);
  } else {
    return delta->dictionaryByTableId(column, table_id);
  }
}

inline Store::table_offset_idx_t Store::responsibleTable(const size_t row) const {
  size_t offset = 0;
  size_t tableIdx = 0;

  for (const auto& table: main_tables) {
    size_t sz = table->size();
    if ((sz + offset) > row) {
      return {table, row - offset, tableIdx};
    } else {
      offset += sz;
    }
    ++tableIdx;
  }

  if ((delta->size() + offset) > row) {
    return {delta, row - offset, tableIdx};
  }
  throw std::out_of_range("Requested row is located beyond store boundaries");
}

void Store::setValueId(const size_t column, const size_t row, ValueId vid) {
  auto location = responsibleTable(row);
  location.table->setValueId(column, location.offset_in_table, vid);
}

ValueId Store::getValueId(const size_t column, const size_t row) const {
  auto location = responsibleTable(row);
  ValueId valueId = location.table->getValueId(column, location.offset_in_table);
  valueId.table = location.table_index;
  return valueId;
}


size_t Store::size() const {
  size_t main_tables_size = hyrise::functional::foldLeft(main_tables, 0ul,
    [](size_t s, const hyrise::storage::atable_ptr_t& t){return s + t->size();});
  return main_tables_size + delta->size();
}

size_t Store::deltaOffset() const {
  return size() - delta->size();
}

size_t Store::columnCount() const {
  return delta->columnCount();
}

unsigned Store::partitionCount() const {
  return main_tables[0]->partitionCount();
}

size_t Store::partitionWidth(const size_t slice) const {
  // TODO we now require that all main tables have the same layout
  return main_tables[0]->partitionWidth(slice);
}


void Store::print(const size_t limit) const {
  PrettyPrinter::print(this, std::cout, "Store", limit, 0);
  // for (size_t main = 0; main < main_tables.size(); main++) {
  //   std::cout << "== Main - Pos:" << main << ", Gen: " << main_tables[main]->generation() << " -" << std::endl;
  //   main_tables[main]->print(limit);
  // }

  // if (delta) {
  //   std::cout << "== Delta:" << std::endl;
  //   delta->print(limit);
  // }
}

void Store::setMerger(TableMerger *_merger) {
  merger = _merger;
}

void Store::setDefaultMerger() {
  auto merger = new TableMerger(new DefaultMergeStrategy(), new SequentialHeapMerger());
  setMerger(merger);
}


void Store::setDelta(hyrise::storage::atable_ptr_t _delta) {
  delta = _delta;
}

hyrise::storage::atable_ptr_t Store::copy() const {
  std::shared_ptr<Store> new_store = std::make_shared<Store>();

  for (size_t i = 0; i < main_tables.size(); ++i) {
    new_store->main_tables.push_back(main_tables[i]->copy());
  }

  new_store->delta = delta->copy();

  if (merger == nullptr) {
    new_store->merger = nullptr;
  } else {
    new_store->merger = merger->copy();
  }

  return new_store;
}


const attr_vectors_t Store::getAttributeVectors(size_t column) const {
  attr_vectors_t tables;
  for (const auto& main: main_tables) {
    const auto& subtables = main->getAttributeVectors(column);
    tables.insert(tables.end(), subtables.begin(), subtables.end());
  }
  const auto& subtables = delta->getAttributeVectors(column);
  tables.insert(tables.end(), subtables.begin(), subtables.end());
  return tables;
}

void Store::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "Store " << this << std::endl;
  for (const auto& m: main_tables) {
    m->debugStructure(level+1);
  }
  delta->debugStructure(level+1);
}

bool Store::isVisibleForTransaction(pos_t pos, hyrise::tx::transaction_cid_t last_commit_id, hyrise::tx::transaction_id_t tid) const {
  if (_tidVector[pos] == tid) {
    if (last_commit_id >= _cidBeginVector[pos]) {
      // row was inserted and committed by another transaction, then deleted by our transaction
      // if we have a lock for it but someone else committed a delete, something is wrong
      assert(_cidEndVector[pos] == hyrise::tx::INF_CID);
      return false;
    } else {
      // we inserted this row - nobody should have deleted it yet
      assert(_cidEndVector[pos] == hyrise::tx::INF_CID);
      return true;
    }
  } else {
    if (last_commit_id >= _cidBeginVector[pos]) {
      // we are looking at a row that was inserted and deleted before we started - we should see it unless it was already deleted again
      if(last_commit_id >= _cidEndVector[pos]) {
        // the row was deleted and the delete was committed before we started our transaction
        return false;
      } else {
        // the row was deleted but the commit happened after we started our transaction (or the delete was not committed yet)
        return true;
      }
    } else {
      // we are looking at a row that was inserted after we started
      assert(_cidEndVector[pos] > last_commit_id);
      return false;
    }
  }
}

// This method iterates of the pos list and validates each position
void Store::validatePositions(pos_list_t& pos, hyrise::tx::transaction_cid_t last_commit_id, hyrise::tx::transaction_id_t tid) const {
  // Make sure we captured all rows
  assert(_cidBeginVector.size() == size() && _cidEndVector.size() == size() && _tidVector.size() == size());

  // Pos is nullptr, we should circumvent
  auto end = std::remove_if(std::begin(pos), std::end(pos), [&](const pos_t& v){

    return !isVisibleForTransaction(v, last_commit_id, tid);

  } );
  if (end != pos.end())
    pos.erase(end, pos.end());
}

pos_list_t Store::buildValidPositions(hyrise::tx::transaction_cid_t last_commit_id, hyrise::tx::transaction_id_t tid) const {
  pos_list_t result;
  hyrise::functional::forEachWithIndex(_cidBeginVector, [&](size_t i, bool v){
    if(isVisibleForTransaction(i, last_commit_id, tid)) result.push_back(i);
  });
  return result;
}

std::pair<size_t, size_t> Store::resizeDelta(size_t num) {
  assert(num > delta->size());
  return appendToDelta(num - delta->size());
}

std::pair<size_t, size_t> Store::appendToDelta(size_t num) {
  static hyrise::locking::Spinlock mtx;
  std::lock_guard<hyrise::locking::Spinlock> lck(mtx);

  size_t start = delta->size();
  std::pair<size_t, size_t> result = {start, start + num};

  // Update Delta
  delta->resize(start + num);
  // Update CID, TID and valid
  auto main_tables_size = hyrise::functional::sum(main_tables, 0ul, [](hyrise::storage::atable_ptr_t& t){return t->size();});
  _cidBeginVector.resize(main_tables_size + start + num, hyrise::tx::INF_CID);
  _cidEndVector.resize(main_tables_size + start + num, hyrise::tx::INF_CID);
  _tidVector.resize(main_tables_size + start + num, hyrise::tx::UNKNOWN);

  return std::move(result);
}

void Store::copyRowToDelta(const hyrise::storage::c_atable_ptr_t& source, const size_t src_row, const size_t dst_row, hyrise::tx::transaction_id_t tid) {
  auto main_tables_size = hyrise::functional::sum(main_tables, 0ul, [](hyrise::storage::atable_ptr_t& t){return t->size();});

  // Update the validity
  _tidVector[main_tables_size + dst_row] = tid;

  delta->copyRowFrom(source, src_row, dst_row, true);
}

hyrise::tx::TX_CODE Store::commitPositions(const pos_list_t& pos, const hyrise::tx::transaction_cid_t cid, bool valid) {
  for(const auto& p : pos) {
    if(valid) {
      _cidBeginVector[p] = cid;
    } else {
      _cidEndVector[p] = cid;
    }
    _tidVector[p] = hyrise::tx::UNKNOWN;
  }
  return hyrise::tx::TX_CODE::TX_OK;
}

hyrise::tx::TX_CODE Store::checkForConcurrentCommit(const pos_list_t& pos, const hyrise::tx::transaction_id_t tid) const {
  for(const auto& p : pos) {
    if (_tidVector[p] != tid)
      return hyrise::tx::TX_CODE::TX_FAIL_CONCURRENT_COMMIT;
  }
  return hyrise::tx::TX_CODE::TX_OK;
}

hyrise::tx::TX_CODE Store::markForDeletion(const pos_t pos, const hyrise::tx::transaction_id_t tid) {
  if(atomic_cas(&_tidVector[pos], hyrise::tx::UNKNOWN, tid)) {
    return hyrise::tx::TX_CODE::TX_OK;
  } else {
    return hyrise::tx::TX_CODE::TX_FAIL_CONCURRENT_COMMIT;
  }
}

hyrise::tx::TX_CODE Store::unmarkForDeletion(const pos_list_t& pos, const hyrise::tx::transaction_id_t tid) {
  for(const auto& p : pos) {
    if (_tidVector[p] == tid)
      _tidVector[p] = hyrise::tx::UNKNOWN;
  }
  return hyrise::tx::TX_CODE::TX_OK;
}
