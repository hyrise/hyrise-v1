// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * TableRangeView.h
 *
 *  Created on: Jan 14, 2013
 *      Author: jwust
 */

#ifndef TABLERANGEVIEW_H_
#define TABLERANGEVIEW_H_

#include "storage/AbstractTable.h"

/*
 * provides an interface to a sequential range of rows to a table defined by a start and end
 */

class TableRangeView : public AbstractTable {

  hyrise::storage::atable_ptr_t _table;

  size_t _start;
  size_t _end;
  // store number of columns to avoid subsequent calls to table->columnCount()
  size_t _columnCount;

public:
  TableRangeView(hyrise::storage::atable_ptr_t t, size_t s, size_t e);
  virtual ~TableRangeView();

  size_t getStart() const;
  hyrise::storage::c_atable_ptr_t getTable() const;
  // specific to TableRangeView
  table_id_t subtableCount() const;
  hyrise::storage::atable_ptr_t copy() const;
  void print(const size_t limit = (size_t) -1) const;

  // recalculated rows and routed to underlying table if necessary
  size_t size() const;
  void setValueId(const size_t column, const size_t row, const ValueId valueId);
  ValueId getValueId(const size_t column, const size_t row) const;
  void *atSlice(const size_t slice, const size_t row) const;
  const ColumnMetadata *metadataAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0) const;
  const SharedDictionaryPtr & dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const;

  // throw exceptions if called
  void setDictionaryAt(SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0);
  void sortDictionary();

  // just routed to underlying table
  size_t getSliceWidth(const size_t slice) const;
  size_t getSliceForColumn(const size_t column) const;
  size_t getOffsetInSlice(const size_t column) const;
  unsigned sliceCount() const;
  hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, const bool reuse_dict = false, const size_t initial_size = 0, const bool with_containers = true, const bool compressed = false) const;
  const SharedDictionaryPtr & dictionaryByTableId(const size_t column, const table_id_t table_id) const;
  DataType typeOfColumn(const size_t column) const;
  size_t columnCount() const;
  std::string nameOfColumn(const size_t column) const;

  virtual void debugStructure(size_t level=0) const;
};

#endif /* TABLERANGEVIEW_H_ */
