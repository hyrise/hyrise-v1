// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_POINTERCALCULATOR_H_
#define SRC_LIB_STORAGE_POINTERCALCULATOR_H_

#include <vector>

#include "helper/types.h"
#include "helper/SharedFactory.h"

#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"

class PointerCalculator : public AbstractTable,
                          public SharedFactory<PointerCalculator> {
public:
  PointerCalculator(hyrise::storage::c_atable_ptr_t t, pos_list_t *pos = nullptr, field_list_t *f = nullptr);
  PointerCalculator(const PointerCalculator& other);
  
  virtual ~PointerCalculator();

  void setPositions(const pos_list_t pos);
  void setFields(const field_list_t f);

  std::shared_ptr<PointerCalculator> intersect(const std::shared_ptr<const PointerCalculator>& other) const;
  std::shared_ptr<PointerCalculator> unite(const std::shared_ptr<const PointerCalculator>& other) const;
  std::shared_ptr<PointerCalculator> concatenate(const std::shared_ptr<const PointerCalculator>& other) const;

  typedef std::vector<std::shared_ptr<const PointerCalculator> > pc_vector;
  static std::shared_ptr<const PointerCalculator> unite_many(pc_vector::const_iterator it, pc_vector::const_iterator it_end);
  static std::shared_ptr<PointerCalculator> concatenate_many(pc_vector::const_iterator it, pc_vector::const_iterator it_end);

  const pos_list_t *getPositions() const;
  pos_list_t getActualTablePositions() const;

  size_t getTableRowForRow(const size_t row) const;
  size_t getTableColumnForColumn(const size_t column) const;

  hyrise::storage::c_atable_ptr_t getTable() const;
  hyrise::storage::c_atable_ptr_t getActualTable() const;

  /**
  * Checks the internal table of the pointer calculator to only contain valid positions.
  *
  * If the PC is used for projections it will create a new position list with
  * all valid positions and if positions are provided will use those positions
  * to validate
  */
  void validate(hyrise::tx::transaction_id_t tid, hyrise::tx::transaction_id_t cid);

  void remove(const pos_list_t& pl);
  
  // AbstractTable interface
  hyrise::storage::atable_ptr_t copy() const override;
  hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, const bool reuse_dict = false, const size_t initial_size = 0, const bool with_containers = true, const bool compressed = false) const override;
  const ColumnMetadata *metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const override;
  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0) const override;
  const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const override;
  void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0) override;
  size_t size() const override;
  size_t columnCount() const override;
  ValueId getValueId(const size_t column, const size_t row) const override;
  unsigned partitionCount() const override;
  size_t partitionWidth(const size_t slice) const override;
  void print(const size_t limit = (size_t) -1) const override;
  table_id_t subtableCount() const override { return 1; }
  void debugStructure(size_t level=0) const override;
 protected:
  void updateFieldMapping();
 private:
  hyrise::storage::c_atable_ptr_t table;
  pos_list_t *pos_list;
  field_list_t *fields;

  std::vector<size_t> slice_for_slice;
  std::vector<size_t> offset_in_slice;
  std::vector<size_t> width_for_slice;
  size_t slice_count;
};

#endif  // SRC_LIB_STORAGE_POINTERCALCULATOR_H_

