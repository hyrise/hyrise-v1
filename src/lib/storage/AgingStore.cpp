// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingStore.h"

#include <set>

#include <storage/MutableHorizontalTable.h>
#include <storage/MutableVerticalTable.h>
#include <storage/Table.h>
#include <storage/StoreRangeView.h>

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

void AgingStore::age(const pos_list_t& posList) {
  if (posList.size() == 0)
    return;

  for (field_t col = 0; col < columnCount(); ++col)
    age(col, posList);
}

void AgingStore::age(field_t field, const pos_list_t& posList) {
  const size_t posSize = posList.size();
  if (posSize == 0)
    return;

  const auto& vtable = checked_pointer_cast<MutableVerticalTable>(getMainTable());
  const auto& htable = checked_pointer_cast<MutableHorizontalTable>(vtable->containerAt(field));
  std::vector<atable_ptr_t> parts(htable->parts().begin(), htable->parts().end());

  std::set<unsigned> effectedPartitions;

  std::vector<std::shared_ptr<AbstractDictionary>> dicts = {parts.front()->dictionaryAt(0)};
  auto metadata = parts.front()->metadata();

  const auto& newHot = std::make_shared<Table>(&metadata, &dicts);
  newHot->resize(posSize);

  for (size_t i = 0; i < posSize; ++i) {
    const auto& pos = posList.at(i);
    const auto& part = htable->partitionAt(pos);
    
    newHot->copyValueFrom(htable, 0, pos, 0, i);
    effectedPartitions.insert(part);
  }

  //posList MUST be sorted
  for (const auto& part : effectedPartitions) {
    const auto& offset = htable->offsetOfPartition(part);
    unsigned i = 0;
    for (; i < posSize; ++i) {
      if (posList.at(i) >= offset)
        break;
    }

    std::vector<size_t> copyRows;
    const size_t size = parts.at(part)->size();
    for (size_t j = 0; j < size; ++j) {
      if (i < posList.size() && posList.at(i) - offset == j) {
        ++i;
        continue;
      }
      copyRows.push_back(j);
    }

    const auto& newCold = std::make_shared<Table>(&metadata, &dicts);
    newCold->resize(copyRows.size());

    for (size_t i = 0; i < copyRows.size(); ++i)
      newCold->copyValueFrom(parts.at(part), 0, copyRows.at(i), 0, i);

    parts.at(part) = newCold;
  }

  parts.insert(parts.begin(), newHot);
  vtable->replacePartition(field, std::make_shared<MutableHorizontalTable>(parts));
}

size_t AgingStore::hotSize(const std::vector<storage::field_t>& fields) const {
  const auto& vtable = checked_pointer_cast<MutableVerticalTable>(getMainTable());

  size_t rowc = 0;
  if (fields.size() != 0) {
    for (const auto& field : fields) {
      const auto& htable = checked_pointer_cast<MutableHorizontalTable>(vtable->containerAt(field));
      rowc = std::max(rowc, htable->parts().at(0)->size());
    }
  }
  else {
    for (unsigned i = 0; i < columnCount(); ++i) {
      const auto& htable = checked_pointer_cast<MutableHorizontalTable>(vtable->containerAt(i));
      rowc = std::max(rowc, htable->parts().at(0)->size());
    }
  }

  return rowc;
}

} }// namespace hyrise::storage

