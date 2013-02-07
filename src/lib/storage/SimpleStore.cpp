// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SimpleStore.h"
#include "TableMerger.h"
#include "SimpleStoreMerger.h"
#include "LogarithmicMergeStrategy.h"

#include <helper/types.h>

namespace hyrise { namespace storage {


void SimpleStore::createDelta() {
  _delta = std::make_shared<delta_table_t>(_main->metadata());
}

SimpleStore::SimpleStore(AbstractTable::SharedTablePtr t) : _main(t) {
  createDelta();
  _merger = std::unique_ptr<TableMerger>(new TableMerger(new LogarithmicMergeStrategy(0), new SimpleStoreMerger()));
}

void SimpleStore::merge() {
  std::vector<hyrise::storage::c_atable_ptr_t> tables { _main, _delta };
  const auto& tmp = _merger->merge(tables);
  _main = tmp[0];
  createDelta();
}

void SimpleStore::mergeWith(std::unique_ptr<TableMerger> merger) {
  std::vector<hyrise::storage::c_atable_ptr_t> tables { _main, _delta };
  const auto& tmp = merger->merge(tables);
  _main = tmp[0];
  createDelta();
}

size_t SimpleStore::size() const {
  return _main->size() + _delta->size();
}

size_t SimpleStore::columnCount() const {
  return _main->columnCount();
}

void SimpleStore::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict,
                     const size_t column, const size_t row, const table_id_t table_id) {
  _main->setDictionaryAt(dict, column, row, table_id);
}

const AbstractTable::SharedDictionaryPtr& SimpleStore::dictionaryByTableId(const size_t column, 
                                                         const table_id_t table_id) const {
  if (table_id > 0) STORAGE_NOT_IMPLEMENTED(SimpleStore, dictionaryByTableId());
  return _main->dictionaryByTableId(column, table_id);
}

const AbstractTable::SharedDictionaryPtr& SimpleStore::dictionaryAt(const size_t column, 
                                                const size_t row, 
                                                const table_id_t table_id, 
                                                const bool of_delta) const {
  if ( row >= _main->size() || table_id > 0 ) STORAGE_NOT_IMPLEMENTED(SimpleStore, dictionaryAt());
  return _main->dictionaryAt(column, row, table_id, of_delta);
}

void SimpleStore::print(const size_t limit) const {
  _main->print();
  _delta->print();
}


ValueId SimpleStore::getValueId(const size_t column, const size_t row) const {
  return _main->getValueId(column, row);
}

void SimpleStore::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  _main->setValueId(column, row, valueId);
}

void *SimpleStore::atSlice(const size_t slice, const size_t row) const {
  return _main->atSlice(slice, row);
}

size_t SimpleStore::getSliceWidth(const size_t slice) const {
  return _main->getSliceWidth(slice);
}

unsigned int SimpleStore::sliceCount() const {
  return _main->sliceCount();
}

size_t SimpleStore::getSliceForColumn(size_t column) const {
  return _main->getSliceForColumn(column);
}

size_t SimpleStore::getOffsetInSlice(size_t c) const {
  return _main->getOffsetInSlice(c);
}

AbstractTable::SharedTablePtr SimpleStore::copy() const {
  STORAGE_NOT_IMPLEMENTED(SimpleStore, copy());
}


}}
