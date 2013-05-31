// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file HorizontalTable.h
 *
 * Contains the class definition of HorizontalTable.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */
#ifndef SRC_LIB_STORAGE_HORIZONTALTABLE_H_
#define SRC_LIB_STORAGE_HORIZONTALTABLE_H_

#include <vector>
#include <string>

#include <helper/types.h>
#include <storage/Table.h>
#include <storage/ColumnMetadata.h>
#include <storage/AbstractTable.h>

/**
 * HorizontalTable implements a horizontal table layout. It is organized into one or
 * more horizontal subtables spreading over a distinct number of rows. The subtables
 * can be any subclass of AbstractTable.
 */
class HorizontalTable : public AbstractTable {
private:
  //* Vector for storing the subtables
  std::vector< hyrise::storage::c_atable_ptr_t > parts;

  //* Size of all subtables
  size_t total_size;

  //* Number of subtables
  size_t part_count;

  //* Offset for each subtable
  std::vector<size_t> offsets;

public:
  explicit HorizontalTable(const std::vector<hyrise::storage::c_atable_ptr_t > &_parts);

  virtual ~HorizontalTable();

  const ColumnMetadata *metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const;

  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const;

  const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const;

  virtual void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0);

  size_t size() const;

  size_t columnCount() const;

  ValueId getValueId(const size_t column, const size_t row) const;

  void setValueId(const size_t column, const size_t row, const ValueId valueId);

  unsigned sliceCount() const;

  virtual void *atSlice(const size_t slice, const size_t row) const;
  virtual size_t getSliceWidth(const size_t slice) const;
  virtual size_t getSliceForColumn(const size_t column) const;
  virtual size_t getOffsetInSlice(const size_t column) const;


  /**
   * Returns the index of the subtable containing a certain row.
   *
   * @param row Index of the row for which to retrieve the part.
   */
  inline size_t partForRow(const size_t row) const {
    size_t part = 0;

    do {
      part++;
    } while (part < part_count && offsets[part] <= row);

    return part - 1;
  }

  virtual table_id_t subtableCount() const {
    return part_count;
  }

  virtual  hyrise::storage::atable_ptr_t copy() const;
  virtual void debugStructure(size_t level=0) const;
};

#endif  // SRC_LIB_STORAGE_HORIZONTALTABLE_H_

