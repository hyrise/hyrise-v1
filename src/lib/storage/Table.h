// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file Table.h
 *
 * Contains the class definition of Table.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "helper/types.h"

#include "storage/ColumnMetadata.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractDictionary.h"

#include "storage/BaseAttributeVector.h"
#include "storage/AttributeVectorFactory.h"

namespace hyrise {
namespace storage {

/**
 * Table is the innermost entity in the table structure. It stores the actual
 * values like a regular table and cannot be splitted further.
 */

class Table : public AbstractTable {
 private:
  typedef std::shared_ptr<AbstractDictionary> SharedDictionary;
  typedef std::vector<SharedDictionary> DictionaryVector;

  typedef Table table_type;
  typedef std::vector<ColumnMetadata> MetadataVector;


  // The shared ptr to the attributes we store inside the table
  typedef BaseAttributeVector<value_id_t> AttributeVector;
  typedef std::shared_ptr<AttributeVector> SharedAttributeVector;

 public:
  /*
    Main constructor for the table class that is called to create a
    new table. The parameters can be used to set the meta data and
    dictionaries for the table. Passing dictionaries allows to reuse
    alread created dictionary data.
  */
  Table(metadata_list* m,
        std::vector<SharedDictionary>* d = nullptr,
        size_t initial_size = 0,
        bool sorted = true,
        bool compressed = true,
        const std::string& tableName = "")
  __attribute__((deprecated("Use Table(meta, attrv, vector-of-dicts instead)")));

  // Construct table from vector of metadata,
  // a storage vector and dictionaries
  // Expects: m.size() == dicts.size()
  Table(std::vector<ColumnMetadata> m, SharedAttributeVector av, std::vector<SharedDictionary> dicts);

  ~Table();

  void setAttributes(SharedAttributeVector b);

  size_t size() const override;

  size_t columnCount() const override;

  ValueId getValueId(const size_t column, const size_t row) const override;

  void setValueId(const size_t column, const size_t row, const ValueId valueId) override;

  void reserve(const size_t nr_of_values) override;

  void resize(const size_t nr_of_values) override;

  const ColumnMetadata& metadataAt(const size_t column_index,
                                   const size_t row_index = 0,
                                   const table_id_t table_id = 0) const override;

  virtual const adict_ptr_t& dictionaryAt(const size_t column,
                                          const size_t row = 0,
                                          const table_id_t table_id = 0) const override;

  virtual const adict_ptr_t& dictionaryByTableId(const size_t column, const table_id_t table_id) const override;

  virtual void setDictionaryAt(adict_ptr_t dict,
                               const size_t column,
                               const size_t row = 0,
                               const table_id_t table_id = 0) override;

  virtual atable_ptr_t copy_structure(const field_list_t* fields = nullptr,
                                      const bool reuse_dict = false,
                                      const size_t initial_size = 0,
                                      const bool with_containers = true,
                                      const bool compressed = false) const override;

  virtual atable_ptr_t copy_structure_modifiable(const field_list_t* fields = nullptr,
                                                 const size_t initial_size = 0,
                                                 const bool with_containers = true) const override;
  virtual atable_ptr_t copy_structure(abstract_dictionary_callback, abstract_attribute_vector_callback) const override;

  unsigned partitionCount() const override { return 1; }

  virtual table_id_t subtableCount() const override { return 1; }

  virtual size_t partitionWidth(const size_t slice) const override { return columnCount(); }

  virtual atable_ptr_t copy() const override;

  virtual const attr_vectors_t getAttributeVectors(size_t column) const override {
    // attr_vector_ref_t tpl_ref(tuples);
    attr_vector_offset_t t{tuples, column};
    return {t};
  }

  void persist_scattered(const pos_list_t& elements, bool new_elements = true) const override;

  virtual void debugStructure(size_t level = 0) const override;

 private:
  enum class DICTIONARY_FLAG {
    CREATE,
    REUSE
  };

  atable_ptr_t copy_structure_common(const std::vector<size_t>& fields_to_copy,
                                     size_t initial_size,
                                     DICTIONARY_FLAG dictionary_policy,
                                     COMPRESSION_FLAG compression,
                                     CONCURRENCY_FLAG concurrency) const;


  //* Attribute Vector
  SharedAttributeVector tuples;

  //* Table structure
  MetadataVector _metadata;

  //* Vector storing the dictionaries
  DictionaryVector _dictionaries;

  //* Number of columns
  size_t width = 0;

  bool _compressed = false;
};
}
}  // namespace hyrise::storage
