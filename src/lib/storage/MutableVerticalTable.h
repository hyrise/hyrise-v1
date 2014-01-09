// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file MutableVerticalTable.h
 *
 * Contains the class definition of MutableVerticalTable.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */
#pragma once

#include <vector>
#include <string>

#include "helper/types.h"

#include "storage/Table.h"
#include "storage/TableFactory.h"
#include "storage/ColumnMetadata.h"
#include "storage/AbstractTable.h"

namespace hyrise { namespace storage {

/**
 * MutableVerticalTable implements a vertical table layout. It is organized into one or
 * more vertical entities spreading over a distinct number of columns. The entities
 * can be any subclass of AbstractTable.
 */
class MutableVerticalTable : public AbstractTable {

public:
  MutableVerticalTable(std::vector<std::vector<ColumnMetadata > *> metadata,
                       std::vector<std::vector<adict_ptr_t> *> *dictionaries = nullptr,
                       size_t size = 0,
                       bool sorted = true,
                       AbstractTableFactory *factory = nullptr,
                       bool compressed = true);
  MutableVerticalTable(std::vector<atable_ptr_t> cs, size_t size = 0);
  virtual ~MutableVerticalTable();
  
  const ColumnMetadata& metadataAt(size_t column_index, size_t row_index=0, table_id_t table_id=0) const override;

  const adict_ptr_t& dictionaryAt(size_t column, size_t row = 0, table_id_t table_id = 0) const override;
  const adict_ptr_t& dictionaryByTableId(size_t column, table_id_t table_id) const override;
  void setDictionaryAt(adict_ptr_t dict, size_t column, size_t row = 0, table_id_t table_id = 0) override;
  size_t size() const override;
  size_t columnCount() const override;
  ValueId getValueId(size_t column, size_t row) const override;
  void setValueId(size_t column, size_t row, ValueId valueId) override;
  void reserve(size_t nr_of_values) override;
  void resize(size_t rows) override;
  unsigned partitionCount() const override;
  size_t partitionWidth(size_t slice) const override;
  atable_ptr_t copy_structure(const field_list_t *fields = nullptr, bool reuse_dict = false, size_t initial_size = 0, bool with_containers = true, bool compressed = false) const override;
  atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, size_t initial_size = 0, bool with_containers = true) const override;
  atable_ptr_t copy_structure(abstract_dictionary_callback, abstract_attribute_vector_callback) const override;
  table_id_t subtableCount() const override;
  atable_ptr_t copy() const override;
  const attr_vectors_t getAttributeVectors(size_t column) const override;
  void debugStructure(size_t level=0) const override;

  /// Returns the container at a given index.
  /// @param container_index Index of the container.
  atable_ptr_t getContainer(size_t container_index) const;
  /// Returns the container for a given column.
  /// @param column_index Index of the column of which to retrieve the container.
  const atable_ptr_t& containerAt(size_t column_index, const bool for_writing = false) const;
 protected:
  /// Returns the offset of a certain column inside its container.
  /// @param column_index Index of the column.
  size_t getOffsetInContainer(size_t column_index) const;
 private:
  /// Vector storing the containers
  std::vector<atable_ptr_t> containers;

  /// Number of columns
  size_t column_count;

  /// Container indices for each column
  std::vector<unsigned> container_for_column;

  /// Offsets of columns in container
  std::vector<size_t> offset_in_container;

  /// Number of slices
  size_t slice_count;

  //* Container indices for each slice
  std::vector<unsigned> container_for_slice;

  //* Offsets of slices in container
  std::vector<unsigned> slice_offset_in_container;
};

}}

