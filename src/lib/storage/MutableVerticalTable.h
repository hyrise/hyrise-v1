// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file MutableVerticalTable.h
 *
 * Contains the class definition of MutableVerticalTable.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */
#ifndef SRC_LIB_STORAGE_VERTICALTABLE_H_
#define SRC_LIB_STORAGE_VERTICALTABLE_H_

#include <vector>
#include <string>

#include "helper/types.h"

#include "storage/Table.h"
#include "storage/TableFactory.h"
#include "storage/ColumnMetadata.h"
#include "storage/AbstractTable.h"

/**
 * MutableVerticalTable implements a vertical table layout. It is organized into one or
 * more vertical entities spreading over a distinct number of columns. The entities
 * can be any subclass of AbstractTable.
 */
class MutableVerticalTable : public AbstractTable {
private:
  //* Vector storing the containers
  std::vector<hyrise::storage::atable_ptr_t> containers;

  //* Number of columns
  size_t column_count;

  //* Container indices for each column
  std::vector<unsigned> container_for_column;

  //* Offsets of columns in container
  std::vector<size_t> offset_in_container;

  //* Number of slices
  size_t slice_count;

  //* Container indices for each slice
  std::vector<unsigned> container_for_slice;

  //* Offsets of slices in container
  std::vector<unsigned> slice_offset_in_container;

public:

  /*
    Constructor of the MutableVerticalTable class.

    Even though the vertical table itself does not carry any data
    this constructor is a helper to easily construct the vertical
    table and the table below.
  */
  MutableVerticalTable(std::vector<std::vector<const ColumnMetadata *> *> metadata,
                std::vector<std::vector<AbstractTable::SharedDictionaryPtr> *> *dictionaries = nullptr,
                size_t size = 0, bool sorted = true, AbstractTableFactory *factory = nullptr, bool compressed = true);

  MutableVerticalTable(std::vector<hyrise::storage::atable_ptr_t> &cs, size_t size = 0);

  MutableVerticalTable() {};

  virtual ~MutableVerticalTable();

  /**
   * Returns the container at a given index.
   *
   * @param container_index Index of the container.
   */
  hyrise::storage::atable_ptr_t getContainer(const size_t container_index) const;

  /**
   * Returns the container for a given column.
   *
   * @param column_index Index of the column of which to retrieve the container.
   */
  const hyrise::storage::atable_ptr_t& containerAt(const size_t column_index, const bool for_writing = false) const;

  /**
   * Returns the offset of a certain column inside its container.
   *
   * @param column_index Index of the column.
   */
  size_t getOffsetInContainer(const size_t column_index) const;

  const ColumnMetadata *metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const;

  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const;

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const;

  virtual void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0);

  size_t size() const;

  size_t columnCount() const;

  ValueId getValueId(const size_t column, const size_t row) const;

  virtual void setValueId(const size_t column, const size_t row, const ValueId valueId);

  void reserve(const size_t nr_of_values);

  void resize(const size_t rows);

  unsigned sliceCount() const;

  virtual void *atSlice(const size_t slice, const size_t row) const;

  virtual size_t getSliceWidth(const size_t slice) const;

  virtual  hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, const bool reuse_dict = false, const size_t initial_size = 0, const bool with_containers = true, const bool compressed = false) const;

  virtual  hyrise::storage::atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, const size_t initial_size = 0, const bool with_containers = true) const;

  virtual table_id_t subtableCount() const {
    return 1;
  }


  virtual  hyrise::storage::atable_ptr_t copy() const;

  virtual size_t getSliceForColumn(const size_t column) const;

  virtual size_t getOffsetInSlice(const size_t column) const;

  virtual const attr_vectors_t getAttributeVectors(size_t column) const;

  void debugStructure(size_t level=0) const;
};

#endif  // SRC_LIB_STORAGE_VERTICALTABLE_H_

