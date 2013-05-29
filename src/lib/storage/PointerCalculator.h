// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_POINTERCALCULATOR_H_
#define SRC_LIB_STORAGE_POINTERCALCULATOR_H_

#include <vector>
#include <sstream>
#include <algorithm>

#include "helper/types.h"

#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"

class PointerCalculator : public AbstractTable,
                          public std::enable_shared_from_this<PointerCalculator> {
private:
  hyrise::storage::c_atable_ptr_t table;
  pos_list_t *pos_list;
  field_list_t *fields;

  std::vector<size_t> slice_for_slice;
  std::vector<size_t> offset_in_slice;
  std::vector<size_t> width_for_slice;
  size_t slice_count;

protected:
  void updateFieldMapping();

public:
  PointerCalculator(hyrise::storage::c_atable_ptr_t t, pos_list_t *pos = nullptr, field_list_t *f = nullptr);
  PointerCalculator(const PointerCalculator& other);
  
  virtual ~PointerCalculator();

  virtual  hyrise::storage::atable_ptr_t copy() const;

  void setPositions(const pos_list_t pos);

  void setFields(const field_list_t f);

  const ColumnMetadata *metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const;

  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const;

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const;

  virtual void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0);

  size_t size() const;

  size_t columnCount() const;

  ValueId getValueId(const size_t column, const size_t row) const;

  unsigned sliceCount() const;

  virtual void *atSlice(const size_t slice, const size_t row) const;

  virtual size_t getSliceWidth(const size_t slice) const;

  virtual size_t getSliceForColumn(const size_t column) const;

  virtual size_t getOffsetInSlice(const size_t column) const;

  void print(const size_t limit = (size_t) -1) const;

  void sortDictionary();

  std::string printValue(const size_t column, const size_t row) const;

  virtual table_id_t subtableCount() const {
    return 1;
  }

  size_t getTableRowForRow(const size_t row) const;

  size_t getTableColumnForColumn(const size_t column) const;

  hyrise::storage::c_atable_ptr_t getTable() const;

  hyrise::storage::c_atable_ptr_t getActualTable() const;

  const pos_list_t *getPositions() const;

  pos_list_t getActualTablePositions() const;

  virtual  hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, const bool reuse_dict = false, const size_t initial_size = 0, const bool with_containers = true, const bool compressed = false) const;


  std::shared_ptr<PointerCalculator> intersect(const std::shared_ptr<const PointerCalculator>& other) const;
  std::shared_ptr<PointerCalculator> unite(const std::shared_ptr<const PointerCalculator>& other) const;
  std::shared_ptr<PointerCalculator> concatenate(const std::shared_ptr<const PointerCalculator>& other) const;

  typedef std::vector<std::shared_ptr<const PointerCalculator> > pc_vector;
  static std::shared_ptr<const PointerCalculator> unite_many(pc_vector::const_iterator it, pc_vector::const_iterator it_end);
  static std::shared_ptr<PointerCalculator> concatenate_many(pc_vector::const_iterator it, pc_vector::const_iterator it_end);

  virtual void debugStructure(size_t level=0) const;
};

#endif  // SRC_LIB_STORAGE_POINTERCALCULATOR_H_

