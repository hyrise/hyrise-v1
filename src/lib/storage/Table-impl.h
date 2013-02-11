// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_TABLE_IMPL_H_
#define SRC_LIB_STORAGE_TABLE_IMPL_H_

#include <cmath>
#include "storage/AttributeVectorFactory.h"

ALLOC_FUNC_TEMPLATE
Table<Strategy, Allocator>::Table(
  std::vector<const ColumnMetadata *> *m,
  std::vector<SharedDictionary> *d,
  size_t initial_size,
  bool sorted,
  size_t padding_size,
  size_t _align_size,
  bool compressed) :
  _metadata(m->size()),
  _dictionaries(m->size()),
  width(m->size()),
  align_size(_align_size),
  _compressed(compressed) {

  // Ownership change for meta data
  for (size_t i = 0; i < width; i++) {
    _metadata[i] = new ColumnMetadata(*m->at(i));
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
        _dictionaries[i] = AbstractDictionary::dictionaryWithType <
                           DictionaryFactory<OrderIndifferentDictionary, Strategy, Allocator> > (
                             _metadata[i]->getType(), initial_size);
      }
    }
  }

  byte_width = width * sizeof(value_id_t);

  if (padding_size > 0) {
    byte_width = byte_width + padding_size - byte_width % padding_size;
  }

  /** Build the attribute vector */
  if (!sorted)
    tuples = AttributeVectorFactory::getAttributeVector<value_id_t, Allocator<value_id_t, Strategy> >(width, initial_size);
  else {
    std::vector<uint64_t> bits(_dictionaries.size(), 0);
    if (d) {
      for (size_t i = 0; i < _dictionaries.size(); ++i)
        bits[i] = _dictionaries[i]->size() == 1 ? 1 : ceil(log(_dictionaries[i]->size()) / log(2.0));
    }
    tuples = AttributeVectorFactory::getAttributeVector2<value_id_t, Allocator<value_id_t, Strategy> >(width, initial_size, compressed, bits);
  }
}


ALLOC_FUNC_TEMPLATE
AbstractTable::SharedTablePtr Table<Strategy, Allocator>::copy_structure(const field_list_t *fields, const bool reuse_dict, const size_t initial_size, const bool with_containers, const bool compressed) const {

  std::vector<const ColumnMetadata *> metadata;
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

  auto res =  std::make_shared<Table<>>(&metadata, dictionaries, initial_size,
                                        true, STORAGE_ALIGNMENT_SIZE, align_size, compressed);
  delete dictionaries;
  return res;

}

ALLOC_FUNC_TEMPLATE
AbstractTable::SharedTablePtr Table<Strategy, Allocator>::copy_structure_modifiable(const field_list_t *fields, const size_t initial_size, const bool with_containers) const {

  std::vector<const ColumnMetadata *> metadata;
  std::vector<AbstractTable::SharedDictionaryPtr > *dictionaries = new std::vector<AbstractTable::SharedDictionaryPtr >;

  if (fields != nullptr) {
for (const field_t & field: *fields) {
      metadata.push_back(metadataAt(field));
      dictionaries->push_back(AbstractDictionary::dictionaryWithType<DictionaryFactory<OrderIndifferentDictionary> >(metadataAt(field)->getType()));
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      metadata.push_back(metadataAt(i));
      dictionaries->push_back(AbstractDictionary::dictionaryWithType<DictionaryFactory<OrderIndifferentDictionary> >(metadataAt(i)->getType()));
    }
  }


  auto result = std::make_shared<Table<>>(&metadata, dictionaries, initial_size, false,
                                          STORAGE_ALIGNMENT_SIZE, align_size, _compressed);
  delete dictionaries;
  return result;

}


ALLOC_FUNC_TEMPLATE
Table<Strategy, Allocator>::~Table() {
for (const auto & m: _metadata) {
    delete m;
  }

}

ALLOC_FUNC_TEMPLATE
size_t Table<Strategy, Allocator>::size() const {
  return tuples->size();
}

ALLOC_FUNC_TEMPLATE
size_t Table<Strategy, Allocator>::columnCount() const {
  return width;
}

ALLOC_FUNC_TEMPLATE
ValueId Table<Strategy, Allocator>::getValueId(const size_t column, const size_t row) const {
  assert(column < width);
  ValueId valueId;
  valueId.valueId = tuples->get(column, row);
  valueId.table = 0;
  return valueId;
}

ALLOC_FUNC_TEMPLATE
void Table<Strategy, Allocator>::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  assert(column < width);
  tuples->set(column, row, valueId.valueId);
}

ALLOC_FUNC_TEMPLATE
void Table<Strategy, Allocator>::reserve(const size_t nr_of_values) {
  if (nr_of_values > 0)
    tuples->reserve(nr_of_values);
}

ALLOC_FUNC_TEMPLATE
void Table<Strategy, Allocator>::resize(const size_t rows) {
  tuples->resize(rows);
}

ALLOC_FUNC_TEMPLATE
const ColumnMetadata *Table<Strategy, Allocator>::metadataAt(const size_t column, const size_t row_index, const table_id_t table_id) const {
  return _metadata.at(column);
}

ALLOC_FUNC_TEMPLATE
const AbstractTable::SharedDictionaryPtr& Table<Strategy, Allocator>::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id, const bool of_delta) const {
  return _dictionaries[column];
}

ALLOC_FUNC_TEMPLATE
const AbstractTable::SharedDictionaryPtr& Table<Strategy, Allocator>::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  return _dictionaries[column];
}

ALLOC_FUNC_TEMPLATE
void Table<Strategy, Allocator>::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row, const table_id_t table_id) {

  // Swap the dictionaries
  if (_dictionaries[column] == nullptr || _dictionaries[column]->size() != dict->size()) {
    // Rewrite the doc vector
    tuples->rewriteColumn(column, dict->size() == 1 ? 1 : ceil(log(dict->size()) / log(2)));
  }
  _dictionaries[column] = dict;
}


ALLOC_FUNC_TEMPLATE
void Table<Strategy, Allocator>::setAttributes(SharedAttributeVector doc) {
  tuples = doc;
}

ALLOC_FUNC_TEMPLATE
AbstractTable::SharedTablePtr Table<Strategy, Allocator>::copy() const {
  auto new_table = std::make_shared<table_type>(new std::vector<const ColumnMetadata *>(_metadata.begin(), _metadata.end()));

  new_table->width = width;
  new_table->byte_width = byte_width;
  new_table->align_size = align_size;
  new_table->setGeneration(this->generation());

  SharedAttributeVector new_tuples = tuples->copy();
  new_table->setAttributes(new_tuples);

  // copy dictionaries
  for (size_t dictIndex = 0; dictIndex < _dictionaries.size(); ++dictIndex) {
    auto new_dict = _dictionaries[dictIndex]->copy();
    new_table->setDictionaryAt(new_dict, dictIndex);
  }

  return new_table;
}


#endif  // SRC_LIB_STORAGE_TABLE_IMPL_H_

