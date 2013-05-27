// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/HorizontalTable.h"

HorizontalTable::HorizontalTable(const std::vector< hyrise::storage::c_atable_ptr_t >& _parts) : parts(_parts) {
  part_count = parts.size();
  offsets.resize(part_count);
  total_size = 0;
  size_t i = 0;
  for (const auto & part: parts) {
    offsets[i++] = total_size;
    total_size += part->size();
  }
}

HorizontalTable::~HorizontalTable() {
}

const ColumnMetadata *HorizontalTable::metadataAt(const size_t column_index, const size_t row, const table_id_t table_id) const {
  return parts[table_id]->metadataAt(column_index);
}

const AbstractTable::SharedDictionaryPtr& HorizontalTable::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id, const bool of_delta) const {
  size_t part = partForRow(row);
  return parts[part]->dictionaryAt(column, row - offsets[part], table_id, of_delta);
}

const AbstractTable::SharedDictionaryPtr& HorizontalTable::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  return parts[table_id]->dictionaryByTableId(column, 0);
}

void HorizontalTable::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row, const table_id_t table_id) {
  throw std::runtime_error("Cannot set dictionary for HorizontalTable");
}

size_t HorizontalTable::size() const {
  return total_size;
}

size_t HorizontalTable::columnCount() const {
  return parts[0]->columnCount();
}

ValueId HorizontalTable::getValueId(const size_t column, const size_t row) const {
  size_t part = partForRow(row);
  size_t row_w = row - offsets[part];
  ValueId valueId = parts[part]->getValueId(column, row_w);
  valueId.table = part;
  return valueId;
}

void HorizontalTable::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  throw std::runtime_error("Cannot set ValueId for HorizontalTable");
}

unsigned HorizontalTable::sliceCount() const {
  return parts[0]->sliceCount();
}

void *HorizontalTable::atSlice(const size_t slice, const size_t row) const {
  size_t part = partForRow(row);
  return parts[part]->atSlice(slice, row - offsets[part]);
}

size_t HorizontalTable::getSliceWidth(const size_t slice) const {
  return parts[0]->getSliceWidth(slice);
}

size_t HorizontalTable::getSliceForColumn(const size_t column) const {
  throw std::runtime_error("Not implemented");
};

size_t HorizontalTable::getOffsetInSlice(const size_t column) const {
  throw std::runtime_error("Not implemented");
};

hyrise::storage::atable_ptr_t HorizontalTable::copy() const {
  throw std::runtime_error("Not implemented");
}


void HorizontalTable::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "HorizontalTable " << this << std::endl;
  for (const auto& p: parts) {
    p->debugStructure(level+1);
  }
}
