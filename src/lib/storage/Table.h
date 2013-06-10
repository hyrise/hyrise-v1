// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file Table.h
 *
 * Contains the class definition of Table.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */
#ifndef SRC_LIB_STORAGE_TABLE_H_
#define SRC_LIB_STORAGE_TABLE_H_

#include <string>
#include <vector>
#include <memory>

#include "helper/types.h"

#include "storage/ColumnMetadata.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractDictionary.h"
#include "storage/AbstractAllocatedTable.h"

#include "storage/BaseAttributeVector.h"
#include "storage/AttributeVectorFactory.h"

#define STORAGE_ALIGNMENT_SIZE 0

/**
 * Table is the innermost entity in the table structure. It stores the actual
 * values like a regular table and cannot be splitted further.
 */

ALLOC_CLASS(Table) {
private:

  // Typedefs for all dependent types
  typedef std::shared_ptr<AbstractDictionary> SharedDictionary;
  typedef std::vector<SharedDictionary, Allocator<SharedDictionary , Strategy> > DictionaryVector;

  typedef Table<Strategy, Allocator> table_type;
  typedef std::vector<const ColumnMetadata *, Allocator<const ColumnMetadata * , Strategy> > MetadataVector;


  // The shared ptr to the attributes we store inside the table
  typedef BaseAttributeVector<value_id_t> AttributeVector;
  typedef std::shared_ptr<AttributeVector> SharedAttributeVector;


  //* Attribute Vector
  SharedAttributeVector tuples;

  //* Table structure
  MetadataVector _metadata;

  //* Vector storing the dictionaries
  DictionaryVector _dictionaries;

  //* Number of columns
  size_t width;

  //* Tuple width in bytes
  size_t byte_width;

  //* Aligned size
  size_t align_size;

  bool _compressed;

public:

  /*
    Main constructor for the table class that is called to create a
    new table. The parameters can be used to set the meta data and
    dictionaries for the table. Passing dictionaries allows to reuse
    alread created dictionary data.
  */
  Table(metadata_list *m,
        std::vector<SharedDictionary> *d = nullptr,
        size_t initial_size = 0,
        bool sorted = true,
        size_t padding_size = STORAGE_ALIGNMENT_SIZE,
        size_t _align_size = STORAGE_ALIGNMENT_SIZE,
        bool compressed = true);

  Table(): width(0), byte_width(0), align_size(0), _compressed(false) {
  };

  ~Table();

  size_t size() const;

  size_t columnCount() const;

  ValueId getValueId(const size_t column, const size_t row) const;

  void setValueId(const size_t column, const size_t row, const ValueId valueId);

  void reserve(const size_t nr_of_values);

  void resize(const size_t nr_of_values);

  virtual const ColumnMetadata *metadataAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0) const;

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const;

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const;

  virtual void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0);

  virtual  hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, const bool reuse_dict = false, const size_t initial_size = 0, const bool with_containers = true, const bool compressed = false) const;

  virtual  hyrise::storage::atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, const size_t initial_size = 0, const bool with_containers = true) const;


  void setAttributes(SharedAttributeVector b);

  unsigned sliceCount() const {
    return 1;
  }

  virtual table_id_t subtableCount() const {
    return 1;
  }

  virtual void *atSlice(const size_t slice, const size_t row) const {
    return ((value_id_t *) tuples->data()) + row * width;
  }

  virtual size_t getSliceWidth(const size_t slice) const {
    return byte_width;
  }


  virtual hyrise::storage::atable_ptr_t copy() const;

  virtual size_t getSliceForColumn(const size_t column) const {
    return 0;
  }

  virtual size_t getOffsetInSlice(const size_t column) const {
    size_t offset = 0;
    size_t index = 0;
for (const auto & m: _metadata) {
      if (index++ == column) break;
      offset += m->getWidth();
    }
    return offset;
  }


  void setNumRows(size_t s) {
    tuples->setNumRows(s);
  }

  virtual const attr_vectors_t getAttributeVectors(size_t column) const {
    //attr_vector_ref_t tpl_ref(tuples);
    attr_vector_offset_t t { tuples, column};
    return { t };
  }

  virtual void debugStructure(size_t level=0) const {
    std::cout << std::string(level, '\t') << "Table " << this << std::endl;
  }
};

#include "Table-impl.h"

#endif  // SRC_LIB_STORAGE_TABLE_H_

