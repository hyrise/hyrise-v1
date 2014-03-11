// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <string>

#include <helper/types.h>
#include <helper/vector_helpers.h>
#include <storage/ColumnMetadata.h>
#include <storage/AbstractTable.h>

namespace hyrise { namespace storage {

// Horizontally partitioned table layout of n AbstractTable instances.
class MutableHorizontalTable : public AbstractTable {
 public:
  explicit MutableHorizontalTable(std::vector<atable_ptr_t> parts);
  virtual ~MutableHorizontalTable();
  const ColumnMetadata& metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const override;
  const adict_ptr_t& dictionaryAt(size_t column, size_t row=0, table_id_t table_id=0) const override;
  const adict_ptr_t& dictionaryByTableId(size_t column, table_id_t table_id) const override;
  void setDictionaryAt(adict_ptr_t dict, size_t column, size_t row=0, table_id_t table_id=0) override;
  size_t size() const override;
  size_t columnCount() const override;
  ValueId getValueId(size_t column, size_t row) const override;
  void setValueId(size_t column, size_t row, const ValueId valueId) override;
  unsigned partitionCount() const override;
  unsigned partitionAt(size_t row) const;
  size_t offsetOfPartition(unsigned partition) const;
  size_t partitionWidth(size_t slice) const override;
  table_id_t subtableCount() const override;
  atable_ptr_t copy() const override;
  atable_ptr_t copy_structure(abstract_dictionary_callback a,
                              abstract_attribute_vector_callback b) const override;

  void debugStructure(size_t level=0) const override;
  const std::vector<atable_ptr_t>& parts();

 private:
  size_t partForRow(size_t row) const;
  size_t computeSize() const;
  /// subtables
  std::vector<atable_ptr_t> _parts;
  const std::vector<size_t> _offsets;
  const std::vector<table_id_t> _table_id_offsets;
  /// Offset for each subtable

};

}}

