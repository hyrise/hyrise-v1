// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/MutableVerticalTable.h"

#include <iostream>

#include "helper/vector_helpers.h"

namespace hyrise { namespace storage {

MutableVerticalTable::MutableVerticalTable(std::vector<std::vector<ColumnMetadata > *> metadata,
                                           std::vector<std::vector<adict_ptr_t> *> *dictionaries,
                                           size_t size,
                                           bool sorted,
                                           AbstractTableFactory *factory,
                                           bool compressed) {
  for (size_t i = 0; i < metadata.size(); i++) {
    std::vector<AbstractTable::SharedDictionaryPtr> *dict = nullptr;

    if (dictionaries)
      dict = dictionaries->at(i);

    if (factory)
      containers.push_back(factory->generate(metadata[i], dict, size, sorted, compressed));
    else
      containers.push_back(std::make_shared<Table>(metadata[i], dict, size, sorted));
  }

  column_count = 0;

  for (size_t i = 0; i < containers.size(); i++) {
    column_count += containers[i]->columnCount();
  }

  //size_t container = 0;
  slice_count = 0;

  for (size_t i = 0; i < containers.size(); i++) {
    for (size_t j = 0; j < containers[i]->columnCount(); j++) {
      container_for_column.push_back(i);
      offset_in_container.push_back(j);
    }

    for (size_t s = 0; s < containers[i]->partitionCount(); s++) {
      container_for_slice.push_back(i);
      slice_offset_in_container.push_back(s);
      slice_count++;
    }
  }
  reserve(size);
}

MutableVerticalTable::MutableVerticalTable(std::vector< atable_ptr_t > tables, size_t size) : column_count(0) {
  size_t cnum = 0;
  slice_count = 0;

for (const auto & c: tables) {
    containers.push_back(c);
    size_t cc = c->columnCount();

    for (size_t i = 0; i < cc; ++i) {
      container_for_column.push_back(cnum);
      offset_in_container.push_back(i);
    }

    for (size_t s = 0; s < c->partitionCount(); s++) {
      container_for_slice.push_back(cnum);
      slice_offset_in_container.push_back(s);
      slice_count++;
    }

    column_count += c->columnCount();
    ++cnum;
  }
}

MutableVerticalTable::~MutableVerticalTable() {
}

const atable_ptr_t& MutableVerticalTable::containerAt(const size_t column_index, const bool for_writing) const {
  return containers[container_for_column[column_index]];
}

size_t MutableVerticalTable::getOffsetInContainer(const size_t column_index) const {
  return offset_in_container[column_index];
}

const ColumnMetadata& MutableVerticalTable::metadataAt(const size_t column_index, const size_t row_index, const table_id_t table_id) const {
  return containerAt(column_index)->metadataAt(offset_in_container[column_index]);
}

const adict_ptr_t& MutableVerticalTable::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id) const {
  return containerAt(column)->dictionaryAt(offset_in_container[column], row, table_id);
}

const adict_ptr_t& MutableVerticalTable::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  return containerAt(column)->dictionaryByTableId(offset_in_container[column], table_id);
}

void MutableVerticalTable::setDictionaryAt(adict_ptr_t dict, const size_t column, const size_t row, const table_id_t table_id) {
  containerAt(column)->setDictionaryAt(dict, offset_in_container[column], row, table_id);
}

size_t MutableVerticalTable::size() const {
  if (column_count == 0) return 0;
  return containers[0]->size();
}

size_t MutableVerticalTable::columnCount() const {
  return column_count;
}

ValueId MutableVerticalTable::getValueId(const size_t column, const size_t row) const {
  size_t tmp = offset_in_container[column];
  return containerAt(column)->getValueId(tmp, row);
}

void MutableVerticalTable::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  containerAt(column)->setValueId(offset_in_container[column], row, valueId);
}

void MutableVerticalTable::reserve(const size_t nr_of_values) {
for (auto & c : containers)
    c->reserve(nr_of_values);
}

void MutableVerticalTable::resize(const size_t rows) {
  if (rows > 0) {
    for (auto & c : containers)
      c->resize(rows);
  }
}

unsigned MutableVerticalTable::partitionCount() const {
  return slice_count;
}

size_t MutableVerticalTable::partitionWidth(const size_t slice) const {
  return containers[container_for_slice[slice]]->partitionWidth(slice_offset_in_container[slice]);
}

atable_ptr_t MutableVerticalTable::getContainer(const size_t c) const {
  return containers[c];
}

atable_ptr_t MutableVerticalTable::copy_structure(const field_list_t *fields, const bool reuse_dict, const size_t initial_size, const bool with_containers, const bool compressed) const {
  std::vector< atable_ptr_t > new_containers;
  size_t offset = 0;
  size_t i = 0;

  if (!with_containers) {
    return AbstractTable::copy_structure(fields, reuse_dict, initial_size, with_containers, compressed);
  }

  for (size_t c = 0; c < containers.size(); c++) {
    field_list_t temp_field_list;

    if (fields == nullptr)
      while (i < containers[c]->columnCount() + offset) {
        temp_field_list.push_back(i - offset);
        i++;
      }
    else
      while ((i < fields->size()) && (fields->at(i) < containers[c]->columnCount() + offset)) {
        temp_field_list.push_back(fields->at(i) - offset);
        i++;
      }

    offset += containers[c]->columnCount();

    if (!temp_field_list.empty()) {
      atable_ptr_t new_table = containers[c]->copy_structure(&temp_field_list, reuse_dict, initial_size, with_containers, compressed);
      new_containers.push_back(new_table);
    }
  }


  atable_ptr_t r = std::make_shared<MutableVerticalTable>(new_containers, initial_size);
  return r;
}

atable_ptr_t MutableVerticalTable::copy_structure_modifiable(const field_list_t *fields, const size_t initial_size, const bool with_containers) const {
  std::vector< atable_ptr_t > new_containers;
  size_t offset = 0;
  size_t i = 0;

  if (!with_containers) {
    return AbstractTable::copy_structure_modifiable(fields, initial_size, with_containers);
  }

  for (size_t c = 0; c < containers.size(); c++) {
    field_list_t temp_field_list;

    if (fields == nullptr)
      while (i < containers[c]->columnCount() + offset) {
        temp_field_list.push_back(i - offset);
        i++;
      }
    else
      while ((i < fields->size()) && (fields->at(i) < containers[c]->columnCount() + offset)) {
        temp_field_list.push_back(fields->at(i) - offset);
        i++;
      }

    offset += containers[c]->columnCount();

    if (!temp_field_list.empty()) {
      atable_ptr_t new_table = containers[c]->copy_structure_modifiable(&temp_field_list, initial_size, with_containers);
      new_containers.push_back(new_table);
    }
  }

  atable_ptr_t r = std::make_shared<MutableVerticalTable>(new_containers, initial_size);
  return r;
}

atable_ptr_t MutableVerticalTable::copy_structure(abstract_dictionary_callback a, abstract_attribute_vector_callback b) const {
  auto new_containers = functional::collect(containers, [&](const atable_ptr_t& t) { return t->copy_structure(a, b); });
  return std::make_shared<MutableVerticalTable>(new_containers);
}

table_id_t MutableVerticalTable::subtableCount() const {
  return 1;
}

atable_ptr_t MutableVerticalTable::copy() const {
  // copy containers
  std::vector< atable_ptr_t > cs;

  for (size_t i = 0; i < containers.size(); ++i) {
    cs.push_back(containers[i]->copy());
  }

  auto new_table = std::make_shared<MutableVerticalTable>(cs);

  return new_table;
}

const attr_vectors_t MutableVerticalTable::getAttributeVectors(size_t column) const {  
  return containerAt(column)->getAttributeVectors(offset_in_container[column]);
}

void MutableVerticalTable::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "MutableVerticalTable" << this << std::endl;
  for(const auto& c: containers) {
    c->debugStructure(level+1);
  }
}

}}
