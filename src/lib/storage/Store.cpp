// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/Store.h>
#include <iostream>
#include <map>

#include <io/TransactionManager.h>
#include <io/StorageManager.h>
#include <storage/storage_types.h>
#include <storage/PrettyPrinter.h>
#include <storage/DeltaIndex.h>
#include <storage/meta_storage.h>
#include <storage/storage_types.h>

#include <helper/vector_helpers.h>
#include <helper/locking.h>
#include <helper/cas.h>

#include "storage/DictionaryFactory.h"
#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/ConcurrentFixedLengthVector.h"
#include "storage/CompoundValueKeyBuilder.h"


#define DELTA_SIZE_DEFAULT_MAX 10000000
#define REUSE_MAIN_DICTS

namespace hyrise {
namespace storage {

TableMerger* createDefaultMerger() {
  return new TableMerger(new DefaultMergeStrategy, new SequentialHeapMerger, false);
}

Store::Store() : merger(createDefaultMerger()) { setUuid(); }

namespace {

auto create_concurrent_dict = [](DataType dt) { return makeDictionary(types::getConcurrentType(dt)); };
auto create_concurrent_storage = [](std::size_t cols) {
  return std::make_shared<ConcurrentFixedLengthVector<value_id_t>>(cols, 0);
};
}

Store::Store(atable_ptr_t main_table)
    : _delta_size(0),
      _main_table(main_table),
      merger(createDefaultMerger()),
      delta(main_table->copy_structure(create_concurrent_dict, create_concurrent_storage)),
      _cidBeginVector(main_table->size(), 0),
      _cidEndVector(main_table->size(), tx::INF_CID),
      _tidVector(main_table->size(), tx::UNKNOWN) {
  setUuid();
}

Store::Store(const std::string& tableName, atable_ptr_t main_table)
    : _delta_size(0),
      _main_table(main_table),
      merger(createDefaultMerger()),
      delta(main_table->copy_structure(create_concurrent_dict, create_concurrent_storage)),
      _cidBeginVector(main_table->size(), 0),
      _cidEndVector(main_table->size(), tx::INF_CID),
      _tidVector(main_table->size(), tx::UNKNOWN) {
  setUuid();
  setName(tableName);
}


Store::~Store() { delete merger; }

void Store::merge() {
  if (merger == nullptr) {
    throw std::runtime_error("No Merger set.");
  }

  // Create new delta and merge
  atable_ptr_t new_delta = delta->copy_structure(create_concurrent_dict, create_concurrent_storage);
  new_delta->setName(getName());
  if (loggingEnabled())
    new_delta->enableLogging();

  // Prepare the merge
  std::vector<c_atable_ptr_t> tmp{_main_table, delta};

  // get valid positions
  std::vector<bool> validPositions(_cidBeginVector.size());
  tx::transaction_cid_t last_commit_id = tx::TransactionManager::getInstance().getLastCommitId();
  functional::forEachWithIndex(_cidBeginVector, [&](size_t i, bool v) {
    validPositions[i] = isVisibleForTransaction(i, last_commit_id, tx::MERGE_TID);
  });

  auto tables = merger->merge(tmp, true, validPositions, getName());
  assert(tables.size() == 1);
  _main_table = tables.front();

  // Fixup the cid and tid vectors
  _cidBeginVector = tbb::concurrent_vector<tx::transaction_cid_t>(_main_table->size(), tx::UNKNOWN_CID);
  _cidEndVector = tbb::concurrent_vector<tx::transaction_cid_t>(_main_table->size(), tx::INF_CID);
  _tidVector = tbb::concurrent_vector<tx::transaction_id_t>(_main_table->size(), tx::START_TID);

#ifdef REUSE_MAIN_DICTS
  // copy merged main's dictionaries for delta
  for (size_t column = 0; column < columnCount(); ++column) {
    const AbstractDictionary* dict = _main_table->dictionaryAt(column).get();
    switch (typeOfColumn(column)) {
      case IntegerType:
      case IntegerTypeDelta:
      case IntegerTypeDeltaConcurrent:
        // case IntegerNoDictType:
        new_delta->setDictionaryAt(std::make_shared<ConcurrentUnorderedDictionary<hyrise_int_t>>(
                                       ((OrderPreservingDictionary<hyrise_int_t>*)dict)->getValueList()),
                                   column);
        break;
      case FloatType:
      case FloatTypeDelta:
      case FloatTypeDeltaConcurrent:
        // case FloatNoDictType:
        new_delta->setDictionaryAt(std::make_shared<ConcurrentUnorderedDictionary<hyrise_float_t>>(
                                       ((OrderPreservingDictionary<hyrise_float_t>*)dict)->getValueList()),
                                   column);
        break;
      case StringType:
      case StringTypeDelta:
      case StringTypeDeltaConcurrent:
        new_delta->setDictionaryAt(std::make_shared<ConcurrentUnorderedDictionary<hyrise_string_t>>(
                                       ((OrderPreservingDictionary<hyrise_string_t>*)dict)->getValueList()),
                                   column);
        break;
      case IntegerNoDictType:
      case FloatNoDictType:
        break;
    }
  }
#endif

  // Replace the delta partition
  delta = new_delta;
  _delta_size = 0;

// if enabled, persist new main onto disk
#ifdef PERSISTENCY_BUFFEREDLOGGER
  auto* sm = io::StorageManager::getInstance();
  if (loggingEnabled() && sm->exists(getName())) {
    sm->persistTable(getName());
  }
#endif
}


atable_ptr_t Store::getMainTable() const { return _main_table; }

atable_ptr_t Store::getDeltaTable() const { return delta; }

const ColumnMetadata& Store::metadataAt(const size_t column_index,
                                        const size_t row_index,
                                        const table_id_t table_id) const {
  size_t offset = _main_table->size();
  if (row_index < offset) {
    return _main_table->metadataAt(column_index, row_index, table_id);
  }
  return delta->metadataAt(column_index, row_index - offset, table_id);
}

void Store::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict,
                            const size_t column,
                            const size_t row,
                            const table_id_t table_id) {
  size_t offset = _main_table->size();
  if (row < offset) {
    _main_table->setDictionaryAt(dict, column, row, table_id);
  }
  delta->setDictionaryAt(dict, column, row - offset, table_id);
}

const AbstractTable::SharedDictionaryPtr& Store::dictionaryAt(const size_t column,
                                                              const size_t row,
                                                              const table_id_t table_id) const {
  size_t offset = _main_table->size();
  if (row < offset) {
    return _main_table->dictionaryAt(column, row);
  }
  return delta->dictionaryAt(column, row - offset);
}

const AbstractTable::SharedDictionaryPtr& Store::dictionaryByTableId(const size_t column,
                                                                     const table_id_t table_id) const {
  if (table_id == 0)
    return _main_table->dictionaryByTableId(column, table_id);
  else
    return delta->dictionaryByTableId(column, table_id);
}

inline Store::table_offset_idx_t Store::responsibleTable(const size_t row) const {
  size_t offset = _main_table->size();
  if (row < offset) {
    return {_main_table, row, 0};
  }
  assert(row - offset < delta->size());
  return {delta, row - offset, 1};
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


size_t Store::size() const { return _main_table->size() + delta->size(); }

size_t Store::deltaOffset() const { return _main_table->size(); }

size_t Store::columnCount() const { return delta->columnCount(); }

unsigned Store::partitionCount() const { return _main_table->partitionCount(); }

size_t Store::partitionWidth(const size_t slice) const { return _main_table->partitionWidth(slice); }


void Store::print_offset(const size_t limit, const size_t offset) const {
  PrettyPrinter::print(this, std::cout, "Store:" + _name, limit, offset);
}

void Store::setMerger(TableMerger* _merger) {
  delete merger;
  merger = _merger;
}

void Store::setDelta(atable_ptr_t _delta) {
  delta = _delta;
  _delta_size = delta->size();
  size_t new_size = this->size();
  _cidBeginVector.resize(new_size, tx::INF_CID);
  _cidEndVector.resize(new_size, tx::INF_CID);
  _tidVector.resize(new_size, tx::START_TID);
  if (loggingEnabled()) {
    _delta->enableLogging();
  }
}

atable_ptr_t Store::copy() const {
  std::shared_ptr<Store> new_store = std::make_shared<Store>();
  new_store->_main_table = _main_table->copy();
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

  const auto& subtablesM = _main_table->getAttributeVectors(column);
  tables.insert(tables.end(), subtablesM.begin(), subtablesM.end());

  const auto& subtables = delta->getAttributeVectors(column);
  tables.insert(tables.end(), subtables.begin(), subtables.end());
  return tables;
}

void Store::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "Store " << this << std::endl;
  std::cout << std::string(level, '\t') << "(main) " << this << std::endl;
  _main_table->debugStructure(level + 1);
  std::cout << std::string(level, '\t') << "(delta) " << this << std::endl;
  delta->debugStructure(level + 1);
}

bool Store::isVisibleForTransaction(pos_t pos, tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const {
  return last_commit_id < _cidEndVector[pos] && (last_commit_id >= _cidBeginVector[pos] || _tidVector[pos] == tid);
}

// This method iterates of the pos list and validates each position
void Store::validatePositions(pos_list_t& pos, tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const {
  // Make sure we captured all rows
  // assert(_cidBeginVector.size() == size() && _cidEndVector.size() == size() && _tidVector.size() == size());

  // Pos is nullptr, we should circumvent
  auto end = std::remove_if(
      std::begin(pos), std::end(pos), [&](const pos_t& v) { return !isVisibleForTransaction(v, last_commit_id, tid); });

  if (end != pos.end())
    pos.erase(end, pos.end());
}

pos_list_t Store::buildValidPositions(tx::transaction_cid_t last_commit_id, tx::transaction_id_t tid) const {
  pos_list_t result;
  pos_t mysize = size();
  for (pos_t i = 0; i < mysize; ++i) {
    if (isVisibleForTransaction(i, last_commit_id, tid))
      result.push_back(i);
  }
  return std::move(result);
}

std::pair<size_t, size_t> Store::resizeDelta(size_t num) {
  assert(num > delta->size());
  return appendToDelta(num - delta->size());
}


std::pair<size_t, size_t> Store::appendToDelta(size_t num_rows) {
  static locking::Spinlock append_lock;
  std::lock_guard<locking::Spinlock> lck(append_lock);
  // NOTE: there are some problems if we execute this without above spinlock, which have to be found yet.

  // By atomically drawing a range of rows unique to the calling thread...
  std::size_t prior_delta_size = _delta_size.fetch_add(num_rows);

  delta->resize(prior_delta_size + num_rows);

  auto main_size = _main_table->size();
  auto new_size = main_size + prior_delta_size + num_rows;

  auto grow_and_fill = [=](tbb::concurrent_vector<tx::transaction_id_t>& vector, tx::transaction_id_t value) {
    vector.grow_to_at_least(new_size);
    // ... we can fill the drawn range without interferring with other threads
    std::fill(std::begin(vector) + main_size + prior_delta_size, std::begin(vector) + new_size, value);
  };
  grow_and_fill(_cidBeginVector, tx::INF_CID);
  grow_and_fill(_cidEndVector, tx::INF_CID);
  grow_and_fill(_tidVector, tx::START_TID);
  return {prior_delta_size, prior_delta_size + num_rows};
}


void Store::copyRowToDelta(const c_atable_ptr_t& source,
                           const size_t src_row,
                           const size_t dst_row,
                           tx::transaction_id_t tid) {
  auto main_tables_size = _main_table->size();

#ifdef REUSE_MAIN_DICTS
  bool copy_values = source.get() != this;
#else
  bool copy_values = true;
#endif

  // Update the validity
  _tidVector[main_tables_size + dst_row] = tid;

// assert((_cidEndVector[main_tables_size + dst_row] == 0));
#ifdef DEBUG
  if (_cidEndVector[main_tables_size + dst_row] == 0) {
    throw std::runtime_error("CID-Vector Not Initialized Error. Details:" + std::to_string(main_tables_size) + " - " +
                             std::to_string(dst_row) + " - " + std::to_string(this->size()));
  }
#endif

  delta->copyRowFrom(source, src_row, dst_row, copy_values);
}

void Store::copyRowToDeltaFromJSONVector(const std::vector<Json::Value>& source,
                                         size_t dst_row,
                                         tx::transaction_id_t tid) {
  auto main_tables_size = _main_table->size();
  // Update the validity
  _tidVector[main_tables_size + dst_row] = tid;

  delta->copyRowFromJSONVector(source, dst_row);
}

void Store::copyRowToDeltaFromStringVector(const std::vector<std::string>& source,
                                           size_t dst_row,
                                           tx::transaction_id_t tid) {
  auto main_tables_size = _main_table->size();

  // Update the validity
  _tidVector[main_tables_size + dst_row] = tid;

  delta->copyRowFromStringVector(source, dst_row);
}

void Store::commitPositions(const pos_list_t& pos, const tx::transaction_cid_t cid, bool valid) {
  for (const auto& p : pos) {
    if (valid) {
      _cidBeginVector[p] = cid;
      _tidVector[p] = tx::START_TID;
    } else {
      _cidEndVector[p] = cid;
    }
  }

  // persist_scattered(pos, valid);
}

void Store::revertPositions(const pos_list_t& pos, bool valid) {
  for (const auto& p : pos) {
    if (valid) {
      _cidBeginVector[p] = tx::INF_CID;
    } else {
      _cidEndVector[p] = tx::INF_CID;
    }
  }

  // persist_scattered(pos, valid);
}

tx::TX_CODE Store::checkForConcurrentCommit(const pos_list_t& pos, const tx::transaction_id_t tid) const {
  for (const auto& p : pos) {
    if (_tidVector[p] != tid) {
      return tx::TX_CODE::TX_FAIL_CONCURRENT_COMMIT;
    }
    if (_cidEndVector[p] != tx::INF_CID) {
      return tx::TX_CODE::TX_FAIL_CONCURRENT_COMMIT;
    }
  }
  return tx::TX_CODE::TX_OK;
}

tx::TX_CODE Store::markForDeletion(const pos_t pos, const tx::transaction_id_t tid) {
  if (atomic_cas(&_tidVector[pos], tx::START_TID, tid)) {
    return tx::TX_CODE::TX_OK;
  }

  if (_tidVector[pos] == tid && _cidEndVector[pos] == tx::INF_CID) {
    // It is a row that we inserted ourselves. So we leave it as it is.
    // No need for a CAS here since we already have it "locked"
    // WARNING:
    // This only works as long as inserted pos as committed before deleted.
    // Otherwise we need to remove the position from the inserted list
    return tx::TX_CODE::TX_OK;
  }

  return tx::TX_CODE::TX_FAIL_CONCURRENT_COMMIT;
}

tx::TX_CODE Store::unmarkForDeletion(const pos_list_t& pos, const tx::transaction_id_t tid) {
  for (const auto& p : pos) {
    if (_tidVector[p] == tid)
      _tidVector[p] = tx::START_TID;
  }
  return tx::TX_CODE::TX_OK;
}


void Store::persist_scattered(const pos_list_t& elements, bool new_elements) const {}

void Store::addMainIndex(std::shared_ptr<AbstractIndex> index, std::vector<field_t> columns) {
  _index_lock.lock();
  _main_indices.push_back(std::make_pair(index, columns));
  _index_lock.unlock();
}

void Store::addDeltaIndex(std::shared_ptr<AbstractIndex> index, std::vector<field_t> columns) {
  _index_lock.lock();
  _delta_indices.push_back(std::make_pair(index, columns));
  _index_lock.unlock();
}

struct AddValueToDeltaIndexFunctor {
  typedef bool value_type;

  c_atable_ptr_t _delta;
  std::shared_ptr<AbstractIndex> _index;
  pos_t _row, _row_offset;
  size_t _column;

  AddValueToDeltaIndexFunctor(c_atable_ptr_t delta,
                              std::shared_ptr<AbstractIndex> index,
                              pos_t row,
                              pos_t row_offset,
                              size_t column)
      : _delta(delta), _index(index), _row(row), _row_offset(row_offset), _column(column) {}

  template <typename ValueType>
  value_type operator()() {
    auto idx = std::dynamic_pointer_cast<DeltaIndex<ValueType>>(_index);
    if (!idx)
      throw std::runtime_error("Index on delta of store needs to be of type DeltaIndex");

    ValueType value = _delta->getValue<ValueType>(_column, _row - _row_offset);
    idx->write_lock();
    idx->add(value, _row);
    idx->unlock();
    return true;
  }
};

void Store::addRowToDeltaIndices(pos_t row) {
  // iterate over all delta indices of the store
  // and add the respective new values of the row

  // TODO: needs to makesure index vector is not modfied during iterating over it

  for (auto index_column_pair : _delta_indices) {
    auto index = index_column_pair.first;
    auto columns = index_column_pair.second;
    storage::type_switch<hyrise_basic_types> ts;
    if (columns.size() == 1) {
      AddValueToDeltaIndexFunctor functor(getDeltaTable(), index, row, _main_table->size(), columns[0]);

      ts(typeOfColumn(columns[0]), functor);
    } else {
      CompoundValueKeyBuilder builder;
      for (auto column : columns) {
        AddValueToCompoundKeyFunctor functor(builder, getDeltaTable().get(), row - _main_table->size(), column);
        ts(typeOfColumn(column), functor);
      }

      auto delta_index = std::dynamic_pointer_cast<DeltaIndex<compound_value_key_t>>(index);
      delta_index->write_lock();
      delta_index->add(builder.get(), row);
      delta_index->unlock();
    }
  }
}

std::vector<std::vector<size_t>> Store::getIndexedColumns() const {
  std::vector<std::vector<size_t>> indexedColumns;
  for (auto idxColPair : _main_indices) {
    indexedColumns.push_back(idxColPair.second);
  }
  return indexedColumns;
}

void Store::enableLogging() {
  logging = true;
  _main_table->enableLogging();
  delta->enableLogging();
}

void Store::setName(const std::string name) {
  _name = name;
  if (_main_table)
    _main_table->setName(name);
  if (delta)
    delta->setName(name);
}


atable_ptr_t Store::copy_structure(abstract_dictionary_callback c, abstract_attribute_vector_callback a) const {
  return _main_table->copy_structure(c, a);
}

}
}
