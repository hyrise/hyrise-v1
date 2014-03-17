// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "MutableHorizontalTable.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

namespace hyrise { namespace storage {

static std::vector<size_t> offsetsFromParts(const std::vector<atable_ptr_t>& parts) {
  std::vector<size_t> offsets(parts.size());
  auto total_size = 0;
  size_t i = 0;
  for (const auto& part: parts) {
    offsets[i++] = total_size;
    total_size += part->size();
  }
  return offsets;
}


static std::vector<table_id_t> tableIdOffsets(const std::vector<atable_ptr_t>& parts) {
  std::vector<table_id_t> offsets(parts.size()+1, 0);
  std::transform(std::begin(parts), std::end(parts),
                 std::begin(offsets) + 1, 
                 [] (const atable_ptr_t& part) {
                     return part->subtableCount();
                 });
  std::partial_sum(std::begin(offsets), std::end(offsets), std::begin(offsets));
  return offsets;
}

MutableHorizontalTable::MutableHorizontalTable(std::vector<atable_ptr_t> parts)
    : _parts(parts), _offsets(offsetsFromParts(_parts)), _table_id_offsets(tableIdOffsets(parts)) {
  assert(_parts.size() != 0);
}

MutableHorizontalTable::~MutableHorizontalTable() = default;

inline size_t MutableHorizontalTable::partForRow(const size_t row) const {
  auto r = std::find_if(std::begin(_offsets), std::end(_offsets),
                        [=] (size_t offset) { return offset > row; });
  return std::distance(std::begin(_offsets), r) - 1;
}

const ColumnMetadata& MutableHorizontalTable::metadataAt(const size_t column_index, const size_t row, const table_id_t table_id) const {
  return _parts[table_id]->metadataAt(column_index);
}

const adict_ptr_t& MutableHorizontalTable::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id) const {
  size_t part = partForRow(row);
  return _parts[part]->dictionaryAt(column, row - _offsets[part], table_id);
}

const adict_ptr_t& MutableHorizontalTable::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  return _parts[table_id]->dictionaryByTableId(column, 0);
}

void MutableHorizontalTable::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row, const table_id_t table_id) {
  throw std::runtime_error("Cannot set dictionary for MutableHorizontalTable");
}

size_t MutableHorizontalTable::size() const {
  return computeSize();
}

size_t MutableHorizontalTable::columnCount() const {
  return _parts[0]->columnCount();
}

ValueId MutableHorizontalTable::getValueId(const size_t column, const size_t row) const {
  size_t part = partForRow(row);
  ValueId valueId = _parts[part]->getValueId(column, row - _offsets[part]);
  valueId.table = _table_id_offsets[part] + valueId.table;
  return valueId;
}

void MutableHorizontalTable::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  unsigned subtable = 0, offset = 0;
  for (; subtable < subtableCount(); ++subtable) {
    if (offset + _parts[subtable]->size() >= row)
      break;
    offset += _parts[subtable]->size();
  }
  if (subtable >= subtableCount())
    throw std::runtime_error("row number exeeds table size");
  _parts[subtable]->setValueId(column, row - offset, valueId);
}

unsigned MutableHorizontalTable::partitionCount() const {
  return _parts[0]->partitionCount();
}

table_id_t MutableHorizontalTable::subtableCount() const {
  return _table_id_offsets.back();
}

unsigned MutableHorizontalTable::partitionAt(size_t row) const {
  unsigned part = 0;
  size_t curOffset = 0;
  while (true) {
     const size_t size = _parts.at(part)->size();
     if (size + curOffset >= row)
       break;
     ++part;
     curOffset += size;
  }
  return part;
}

size_t MutableHorizontalTable::offsetOfPartition(unsigned partition) const {
  if (partition == 0)
    return 0;

  size_t offset = 0;
  for (unsigned i = 0; i < partition - 1; ++i)
    offset+= _parts.at(i)->size();
  return offset;
}

size_t MutableHorizontalTable::partitionWidth(const size_t slice) const {
  return _parts[0]->partitionWidth(slice);
}

atable_ptr_t MutableHorizontalTable::copy() const {
  std::vector<atable_ptr_t> subtables;
  for (unsigned i = 0; i < subtableCount(); ++i)
    subtables.push_back(_parts[i]->copy());
  return std::make_shared<MutableHorizontalTable>(subtables);
}

atable_ptr_t MutableHorizontalTable::copy_structure(abstract_dictionary_callback a,
                              abstract_attribute_vector_callback b) const {
    const auto parts = functional::collect(_parts, [&](const atable_ptr_t& t) { return t->copy_structure(a, b); });
    return std::make_shared<MutableHorizontalTable>(parts);
}

void MutableHorizontalTable::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "MutableHorizontalTable " << this << std::endl;
  for (const auto& p: _parts) {
    p->debugStructure(level+1);
  }
}

size_t MutableHorizontalTable::computeSize() const {
  return std::accumulate(_parts.begin(),
                         _parts.end(),
                         0,
                         [] (size_t r, const atable_ptr_t& t) { return r + t->size(); });
}

const std::vector<atable_ptr_t>& MutableHorizontalTable::parts() {
  return _parts;
}

}}
