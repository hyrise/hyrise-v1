// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/Table.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include "storage/AttributeVectorFactory.h"
#include "storage/DictionaryFactory.h"
#include "storage/ValueIdMap.hpp"

namespace hyrise {
namespace storage {

Table::Table(
  std::vector<ColumnMetadata > *m,
  std::vector<SharedDictionary> *d,
  size_t initial_size,
  bool sorted,
  bool compressed) :
  _metadata(m->size()),
  _dictionaries(m->size()),
  width(m->size()),
  _compressed(compressed) {

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


  /** Build the attribute vector */
  if (!sorted)
    tuples = AttributeVectorFactory::getAttributeVector<value_id_t>(width, initial_size);
  else {
    std::vector<uint64_t> bits(_dictionaries.size(), 0);
    if (d) {
      for (size_t i = 0; i < _dictionaries.size(); ++i)
        bits[i] = _dictionaries[i]->size() == 1 ? 1 : ceil(log(_dictionaries[i]->size()) / log(2.0));
    }
    tuples = AttributeVectorFactory::getAttributeVector2<value_id_t>(width, initial_size, compressed, bits);
  }
}


Table::Table(std::vector<ColumnMetadata> m,
             SharedAttributeVector av,
             std::vector<SharedDictionary> dicts) :
  tuples(av),
  _metadata(m.size()),
  _dictionaries(dicts),
  width(m.size()) {
  assert(m.size() == dicts.size() && "Metadata size and dictionaries must match");
  for (size_t i = 0; i < width; i++) {
    _metadata[i] = m.at(i);
  }
}


atable_ptr_t Table::copy_structure(const field_list_t *fields, const bool reuse_dict, const size_t initial_size, const bool with_containers, const bool compressed) const {

  std::vector<ColumnMetadata > metadata;
  std::vector<AbstractTable::SharedDictionaryPtr> *dictionaries = nullptr;

  if (reuse_dict) {
    dictionaries = new std::vector<AbstractTable::SharedDictionaryPtr>();
  }

  if (fields != nullptr) {
    for (const field_t & field: *fields) {
      metadata.push_back(metadataAt(field));

      if (dictionaries != nullptr) {
        dictionaries->push_back(dictionaryAt(field));
      }
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      metadata.push_back(metadataAt(i));

      if (dictionaries != nullptr) {
        dictionaries->push_back(dictionaryAt(i));
      }
    }
  }

  auto res =  std::make_shared<Table>(&metadata, dictionaries, initial_size, true, compressed);
  delete dictionaries;
  return res;

}


atable_ptr_t Table::copy_structure_modifiable(const field_list_t *fields, const size_t initial_size, const bool with_containers) const {

  std::vector<ColumnMetadata > metadata;
  std::vector<AbstractTable::SharedDictionaryPtr > *dictionaries = new std::vector<AbstractTable::SharedDictionaryPtr >;

  if (fields != nullptr) {
    for (const field_t & field: *fields) {
      metadata.push_back(metadataAt(field));
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      metadata.push_back(metadataAt(i));
    }
  }

  for (const auto& field: metadata) {
    dictionaries->push_back(makeDictionary(field.getType()));
  }

  auto result = std::make_shared<Table>(&metadata, dictionaries, initial_size, false, _compressed);
  delete dictionaries;
  return result;

}

atable_ptr_t Table::copy_structure(abstract_dictionary_callback ad, abstract_attribute_vector_callback aav) const {
  std::vector<ColumnMetadata> metadata;
  std::vector<AbstractTable::SharedDictionaryPtr > dicts;

  for (size_t i = 0; i < columnCount(); ++i) {
    metadata.push_back(metadataAt(i));
  }

  for (const auto& field: metadata) {
    dicts.push_back(ad(field.getType()));
  }

  return std::make_shared<Table>(metadata,
                                 checked_pointer_cast< BaseAttributeVector<value_id_t> >(aav(metadata.size())),
                                 dicts);
}


Table::~Table() {}


size_t Table::size() const {
  return tuples->size();
}


size_t Table::columnCount() const {
  return width;
}


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


void Table::resize(const size_t rows) {
  tuples->resize(rows);
}


const ColumnMetadata& Table::metadataAt(const size_t column, const size_t row_index, const table_id_t table_id) const {
  return _metadata[column];
}


const AbstractTable::SharedDictionaryPtr& Table::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id) const {
  return _dictionaries[column];
}


const AbstractTable::SharedDictionaryPtr& Table::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  return _dictionaries[column];
}


void Table::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row, const table_id_t table_id) {

  // Swap the dictionaries
  if (_dictionaries[column] == nullptr || _dictionaries[column]->size() != dict->size()) {
    // Rewrite the doc vector
    tuples->rewriteColumn(column, dict->size() == 1 ? 1 : ceil(log(dict->size()) / log(2)));
  }

  // Check if we need to upgrade the type
  if (types::isUnordered(_metadata[column].getType()) && dict->isOrdered() ) {
    _metadata[column].setType(types::getOrderedType(_metadata[column].getType()));
  }

  _dictionaries[column] = dict;
}



void Table::setAttributes(SharedAttributeVector doc) {
  tuples = doc;
}


atable_ptr_t Table::copy() const {
  auto new_table = std::make_shared<table_type>(new std::vector<ColumnMetadata >(_metadata.begin(), _metadata.end()));

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

void Table::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "Table " << this << std::endl;
}

} } // namespace hyrise::storage

