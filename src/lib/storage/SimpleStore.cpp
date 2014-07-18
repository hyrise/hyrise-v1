// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SimpleStore.h"
#include "TableMerger.h"
#include "SimpleStoreMerger.h"
#include "AbstractMergeStrategy.h"

#include <iostream>
#include <helper/types.h>

namespace hyrise {
namespace storage {


void SimpleStore::createDelta() { _delta = std::make_shared<delta_table_t>(_main->metadata()); }

SimpleStore::SimpleStore(atable_ptr_t t) : _main(t) {
  createDelta();
  _merger = std::unique_ptr<TableMerger>(new TableMerger(new DefaultMergeStrategy(), new SimpleStoreMerger()));
}

void SimpleStore::merge() {
  std::vector<c_atable_ptr_t> tables{_main, _delta};
  const auto& tmp = _merger->merge(tables);
  _main = tmp[0];
  createDelta();
}

void SimpleStore::mergeWith(std::unique_ptr<TableMerger> merger) {
  std::vector<c_atable_ptr_t> tables{_main, _delta};
  const auto& tmp = merger->merge(tables);
  _main = tmp[0];
  createDelta();
}

size_t SimpleStore::size() const { return _main->size() + _delta->size(); }

size_t SimpleStore::columnCount() const { return _main->columnCount(); }

void SimpleStore::setDictionaryAt(adict_ptr_t dict, const size_t column, const size_t row) {
  _main->setDictionaryAt(dict, column, row);
}

const adict_ptr_t& SimpleStore::dictionaryAt(const size_t column, const size_t row) const {
  if (row >= _main->size())
    STORAGE_NOT_IMPLEMENTED(SimpleStore, dictionaryAt());
  return _main->dictionaryAt(column, row);
}

void SimpleStore::print(const size_t limit) const {
  _main->print(limit);
  _delta->print(limit);
}


ValueId SimpleStore::getValueId(const size_t column, const size_t row) const { return _main->getValueId(column, row); }

void SimpleStore::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  _main->setValueId(column, row, valueId);
}


size_t SimpleStore::partitionWidth(const size_t slice) const { return _main->partitionWidth(slice); }

unsigned int SimpleStore::partitionCount() const { return _main->partitionCount(); }

atable_ptr_t SimpleStore::copy() const { STORAGE_NOT_IMPLEMENTED(SimpleStore, copy()); }

void SimpleStore::persist_scattered(const pos_list_t& elements, bool new_elements) const {
  _main->persist_scattered(elements, new_elements);
  _delta->persist_scattered(elements, new_elements);
}
}
}
