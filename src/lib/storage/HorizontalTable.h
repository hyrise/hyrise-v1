// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file HorizontalTable.h
 *
 * Contains the class definition of HorizontalTable.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */
#pragma once

#include <vector>
#include <string>

#include "helper/types.h"
#include "storage/ColumnMetadata.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

// Horizontally partitioned table layout of n AbstractTable instances.
class HorizontalTable : public AbstractTable {
 public:
  explicit HorizontalTable(std::vector<c_atable_ptr_t> parts);
  virtual ~HorizontalTable();
  const ColumnMetadata& metadataAt(const size_t column_index, const size_t row_index = 0) const override;
  cpart_t getPart(std::size_t column, std::size_t row) const;
  const adict_ptr_t& dictionaryAt(size_t column, size_t row = 0) const override;
  void setDictionaryAt(adict_ptr_t dict, size_t column, size_t row = 0) override;
  size_t size() const override;
  size_t columnCount() const override;
  ValueId getValueId(size_t column, size_t row) const override;
  void setValueId(size_t column, size_t row, const ValueId valueId) override;
  unsigned partitionCount() const override;
  size_t partitionWidth(size_t slice) const override;
  table_id_t subtableCount() const override;
  atable_ptr_t copy() const override;

  Visitation accept(StorageVisitor&) const override;
  Visitation accept(MutableStorageVisitor&) override;

  virtual void collectParts(std::list<cpart_t>& parts, size_t col_offset, size_t row_offset) const override;

 private:
  size_t partForRow(size_t row) const;
  size_t computeSize() const;
  /// subtables
  const std::vector<c_atable_ptr_t> _parts;
  const std::vector<size_t> _offsets;
  const std::vector<table_id_t> _table_id_offsets;
  /// Offset for each subtable

  void persist_scattered(const pos_list_t& elements, bool new_elements = true) const override;
};
}
}
