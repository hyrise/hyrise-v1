// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/Store.h>
#include <iostream>

#include "storage/PrettyPrinter.h"

Store::Store(std::vector<std::vector<const ColumnMetadata *> *> md) :
  merger(nullptr) {
  throw std::runtime_error("Bad things happende");
}

Store::Store() :
  merger(nullptr) {
}

Store::Store(hyrise::storage::atable_ptr_t main_table) :
  delta(main_table->copy_structure_modifiable()),
  merger(nullptr) {
  main_tables.push_back(main_table);
}

Store::~Store() {
  delete merger;
}

void Store::merge() {
  if (merger == nullptr) {
    throw std::runtime_error("No Merger set.");
  }

  hyrise::storage::atable_ptr_t new_delta = delta->copy_structure_modifiable();
  std::vector<hyrise::storage::c_atable_ptr_t> tmp(main_tables.begin(), main_tables.end());
  tmp.push_back(delta);
  main_tables = merger->merge(tmp);
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

const AbstractTable::SharedDictionaryPtr& Store::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id, const bool of_delta) const {
  if (!row) {
    return this->dictionaryByTableId(column, table_id);
  }

  if (table_id) {
    if (table_id < main_tables.size()) {
      return main_tables[table_id]->dictionaryAt(column, row);
    } else {
      return delta->dictionaryAt(column, row);
    }
  }

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
  size_t main_tables_size = 0;

  for (size_t main = 0; main < main_tables.size(); main++) {
    main_tables_size += main_tables[main]->size();
  }


  return main_tables_size + delta->size();
}

size_t Store::columnCount() const {
  return delta->columnCount();
}

unsigned Store::sliceCount() const {
  return main_tables[0]->sliceCount();
}

void *Store::atSlice(const size_t slice, const size_t row) const {
  size_t offset = 0;

  for (size_t main = 0; main < main_tables.size(); main++)
    if (main_tables[main]->size() + offset > row) {
      return main_tables[main]->atSlice(slice, row - offset);
    } else {
      offset += main_tables[main]->size();
    }

  // row is not in main tables. return slice from delta
  return delta->atSlice(slice, row - offset);
}

size_t Store::getSliceWidth(const size_t slice) const {
  // TODO we now require that all main tables have the same layout
  return main_tables[0]->getSliceWidth(slice);
}


size_t Store::getSliceForColumn(const size_t column) const {
  throw std::runtime_error("Not implemented");
  //return main_tables[0]->getSliceForColumn(column);
}

size_t Store::getOffsetInSlice(const size_t column) const {
  throw std::runtime_error("Not implemented");
  //return main_tables[0]->getOffsetInSlice(column);
};


void Store::print(const size_t limit) const {
  for (size_t main = 0; main < main_tables.size(); main++) {
    std::cout << "== Main - Pos:" << main << ", Gen: " << main_tables[main]->generation() << " -" << std::endl;
    main_tables[main]->print(limit);
  }

  if (delta) {
    std::cout << "== Delta:" << std::endl;
    delta->print(limit);
  }
}

void Store::setMerger(TableMerger *_merger) {
  merger = _merger;
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
