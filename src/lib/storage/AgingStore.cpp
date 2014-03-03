// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingStore.h"

#include <iostream>

#include <storage/MutableHorizontalTable.h>
#include <storage/MutableVerticalTable.h>
#include <storage/Table.h>

namespace hyrise { namespace storage {

AgingStore::AgingStore(const store_ptr_t& store) : AgingStore(*store) {}

AgingStore::AgingStore(const Store& store) :
    Store(makeAgingMain(store.getMainTable())) {}

AgingStore::~AgingStore() {}


atable_ptr_t AgingStore::makeAgingMain(const atable_ptr_t& table) {
  std::vector<atable_ptr_t> tables;
  for (unsigned i = 0; i < table->columnCount(); ++i) {
    std::vector<std::shared_ptr<AbstractDictionary>> dicts = {table->dictionaryAt(i)};
    metadata_list metadata = {table->metadataAt(i)};
    const size_t size = table->size();

    const auto& newTable = std::make_shared<Table>(&metadata, &dicts);
    newTable->resize(size);
    for (size_t row = 0; row < size; ++row)
      newTable->copyValueFrom(table, i, row, 0, row);

    std::vector<atable_ptr_t> parts = {newTable};
    tables.push_back(std::make_shared<MutableHorizontalTable>(parts));
  }
  return std::make_shared<MutableVerticalTable>(tables);
}

void AgingStore::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "AgingStore " << this << std::endl;
  getMainTable()->debugStructure(level + 1);
  getDeltaTable()->debugStructure(level + 1);
}

} }// namespace hyrise::storage

