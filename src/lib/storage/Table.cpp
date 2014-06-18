// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/Table.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "storage/AttributeVectorFactory.h"
#include "storage/DictionaryFactory.h"
#include "storage/ValueIdMap.hpp"


namespace hyrise {
namespace storage {

Table::Table(std::vector<ColumnMetadata>* m,
             std::vector<SharedDictionary>* d,
             size_t initial_size,
             bool sorted,
             bool compressed,
             const std::string& tableName)
    : _metadata(m->size()), _dictionaries(m->size()), width(m->size()), _compressed(compressed) {
  // Ownership change for meta data
  for (size_t i = 0; i < width; i++) {
    _metadata[i] = m->at(i);
  }

  // If we pass dictionaries, reuses them
  if (d) {
    std::copy(d->begin(), d->end(), _dictionaries.begin());
  } else {

    // Otheriwse create new dictionaries based on the met data
    _dictionaries.resize(width);

    // Only create dictionaries if they can be used, sorted dictionaries
    // for empty tables is not useful
    if (!sorted) {
      for (size_t i = 0; i < width; i++) {
        _dictionaries[i] = makeDictionary(_metadata[i].getType(), initial_size);
      }
    }
  }

  std::string id;

  /** Build the attribute vector */
  if (!sorted) {
    if (m->size() == 1) {
      id = tableName + "__delta_col__" + m->at(0).getName();
    }
    tuples = create_attribute_vector(
        width, initial_size, CONCURRENCY_FLAG::CONCURRENT, COMPRESSION_FLAG::UNCOMPRESSED, _dictionaries);
  } else {
    if (m->size() == 1) {
      id = tableName + "__main_col__" + m->at(0).getName();
    }
    tuples = create_attribute_vector(
        width, initial_size, CONCURRENCY_FLAG::NOT_CONCURRENT, COMPRESSION_FLAG::UNCOMPRESSED, _dictionaries);
  }
}


Table::Table(std::vector<ColumnMetadata> m, SharedAttributeVector av, std::vector<SharedDictionary> dicts)
    : tuples(av), _metadata(m), _dictionaries(dicts), width(m.size()) {
  assert(m.size() == dicts.size() && "Metadata size and dictionaries must match");
}

enum class DICTIONARY_FLAG {
  CREATE,
  REUSE
};

atable_ptr_t Table::copy_structure_common(const std::vector<size_t>& fields_to_copy,
                                          size_t initial_size,
                                          DICTIONARY_FLAG dictionary_policy,
                                          COMPRESSION_FLAG compression,
                                          CONCURRENCY_FLAG concurrency) const {
  std::vector<ColumnMetadata> metadata;
  std::vector<adict_ptr_t> dictionaries;
  for (const auto& field : fields_to_copy) {
    metadata.push_back(_metadata.at(field));
    dictionaries.push_back(dictionary_policy == DICTIONARY_FLAG::REUSE ? _dictionaries.at(field)
                                                                       : makeDictionary(_metadata.at(field).getType()));
  }

  auto values = create_attribute_vector(fields_to_copy.size(), initial_size, concurrency, compression, dictionaries);
  return std::make_shared<Table>(metadata, checked_pointer_cast<BaseAttributeVector<value_id_t>>(values), dictionaries);
}

std::vector<size_t> proper_fields(const field_list_t* fields, const size_t max_fields) {
  std::vector<size_t> fields_to_copy;
  if (fields == nullptr) {
    fields_to_copy.resize(max_fields);
    std::iota(fields_to_copy.begin(), fields_to_copy.end(), 0);
  } else {
    fields_to_copy = *fields;
  }
  return fields_to_copy;
}

atable_ptr_t Table::copy_structure(const field_list_t* fields,
                                   const bool reuse_dict,
                                   const size_t initial_size,
                                   const bool with_containers,
                                   const bool compressed) const {
  auto fields_to_copy = proper_fields(fields, _metadata.size());
  return copy_structure_common(fields_to_copy,
                               initial_size,
                               reuse_dict ? DICTIONARY_FLAG::REUSE : DICTIONARY_FLAG::CREATE,
                               compressed ? COMPRESSION_FLAG::COMPRESSED : COMPRESSION_FLAG::UNCOMPRESSED,
                               CONCURRENCY_FLAG::NOT_CONCURRENT);
}


atable_ptr_t Table::copy_structure_modifiable(const field_list_t* fields,
                                              const size_t initial_size,
                                              const bool with_containers) const {
  auto fields_to_copy = proper_fields(fields, _metadata.size());
  return copy_structure_common(fields_to_copy,
                               initial_size,
                               DICTIONARY_FLAG::CREATE,
                               COMPRESSION_FLAG::UNCOMPRESSED,
                               CONCURRENCY_FLAG::CONCURRENT);
}

atable_ptr_t Table::copy_structure(abstract_dictionary_callback ad, abstract_attribute_vector_callback aav) const {
  std::vector<ColumnMetadata> metadata;
  std::vector<adict_ptr_t> dicts;

  for (size_t i = 0; i < columnCount(); ++i) {
    metadata.push_back(metadataAt(i));
  }

  for (const auto& field : metadata) {
    dicts.push_back(ad(field.getType()));
  }

  return std::make_shared<Table>(
      metadata, checked_pointer_cast<BaseAttributeVector<value_id_t>>(aav(metadata.size())), dicts);
}


Table::~Table() {}


size_t Table::size() const { return tuples->size(); }


size_t Table::columnCount() const { return width; }


ValueId Table::getValueId(const size_t column, const size_t row) const {
  assert(column < width);
  ValueId valueId;
  valueId.valueId = tuples->get(column, row);
  valueId.table = 0;
  return valueId;
}


void Table::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  assert(column < width);
  tuples->set(column, row, valueId.valueId);
}


void Table::reserve(const size_t nr_of_values) {
  if (nr_of_values > 0)
    tuples->reserve(nr_of_values);
}


void Table::resize(const size_t rows) { tuples->resize(rows); }


const ColumnMetadata& Table::metadataAt(const size_t column, const size_t row_index, const table_id_t table_id) const {
  return _metadata[column];
}


const adict_ptr_t& Table::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id) const {
  return _dictionaries[column];
}


const adict_ptr_t& Table::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  return _dictionaries[column];
}


void Table::setDictionaryAt(adict_ptr_t dict, const size_t column, const size_t row, const table_id_t table_id) {

  // Swap the dictionaries
  if (_dictionaries[column] == nullptr || _dictionaries[column]->size() != dict->size()) {
    // Rewrite the doc vector
    tuples->rewriteColumn(column, dict->size() == 1 ? 1 : ceil(log(dict->size()) / log(2)));
  }

  // Check if we need to upgrade the type
  if (types::isUnordered(_metadata[column].getType()) && dict->isOrdered()) {
    _metadata[column].setType(types::getOrderedType(_metadata[column].getType()));
  }

  _dictionaries[column] = dict;
}



void Table::setAttributes(SharedAttributeVector doc) { tuples = doc; }


atable_ptr_t Table::copy() const {
  auto new_table = std::make_shared<table_type>(new std::vector<ColumnMetadata>(_metadata.begin(), _metadata.end()));

  new_table->width = width;

  SharedAttributeVector new_tuples = tuples->copy();
  new_table->setAttributes(new_tuples);

  // copy dictionaries
  for (size_t dictIndex = 0; dictIndex < _dictionaries.size(); ++dictIndex) {
    auto new_dict = _dictionaries[dictIndex]->copy();
    new_table->setDictionaryAt(new_dict, dictIndex);
  }

  return new_table;
}

void Table::persist_scattered(const pos_list_t& elements, bool new_elements) const {}

void Table::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "Table " << this << std::endl;
}
}
}  // namespace hyrise::storage
