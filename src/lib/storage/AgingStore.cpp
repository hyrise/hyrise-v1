// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingStore.h"

#include <iostream>

#include <storage/MutableHorizontalTable.h>
#include <storage/MutableVerticalTable.h>
#include <storage/Table.h>

namespace hyrise { namespace storage {

AgingStore::AgingStore(const store_ptr_t& store) : AgingStore(*store) {}

AgingStore::AgingStore(const Store& store) :
    Store(makeAgingMain(store.getMainTable())) {

}

AgingStore::~AgingStore() {}


atable_ptr_t AgingStore::makeAgingMain(const atable_ptr_t& table) {
  const auto& vtable = checked_pointer_cast<MutableVerticalTable>(table);
  std::vector<atable_ptr_t> htables;
  bool createNew = false;
  for (unsigned i = 0; i < vtable->subtableCount(); ++i) {
    const auto& subtable = vtable->getContainer(i);
    auto htable = std::dynamic_pointer_cast<MutableHorizontalTable>(subtable);
    if (htable != nullptr)
      htables.push_back(htable);
    else {
      createNew = true;
      const std::vector<atable_ptr_t> tableList = {checked_pointer_cast<Table>(subtable)};
      htables.push_back(std::make_shared<MutableHorizontalTable>(tableList));
    }
  }

  if (createNew == false) {
    std::cout << "already only MutableHorizontalTables?" << std::endl;
    return vtable;
  }
  return std::make_shared<MutableVerticalTable>(htables);
}

void AgingStore::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "AgingStore " << this << std::endl;
  getMainTable()->debugStructure(level + 1);
  getDeltaTable()->debugStructure(level + 1);
}

} }// namespace hyrise::storage

