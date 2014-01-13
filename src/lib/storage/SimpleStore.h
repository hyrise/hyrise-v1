// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>

#include "storage/AbstractTable.h"
#include "storage/RawTable.h"
#include "storage/storage_types.h"
#include "storage/TableMerger.h"

namespace hyrise { namespace storage {

class SimpleStore : public AbstractTable {

private:

  atable_ptr_t _main;
  std::shared_ptr<RawTable> _delta;

public:

  typedef RawTable delta_table_t;
  typedef AbstractTable main_table_t;

  explicit SimpleStore(atable_ptr_t t);

  virtual ~SimpleStore() {}

  /**
   * @see AbstractTable
   */
  size_t columnCount() const;

  /**
   * @see AbstractTable
   */
  size_t size() const;

  /**
   * @see AbstractTable
   */
  table_id_t subtableCount() const { return 1; }

  /**
   * @see AbstractTable
   */
  const ColumnMetadata& metadataAt(size_t c, size_t r, table_id_t t) const { return _main->metadataAt(c,r,t); }

  /**
   * @see AbstractTable
   */
  atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, 
                                                                   const size_t initial_size = 0, 
                                                                   const bool with_containers = true) const {
    throw std::runtime_error("RawTable ("+ STORAGE_DEBUG_WHERE_WHAT(__FILE__, __LINE__) +") does not support copy structure modifiable"); 
  }

  template <typename T>
  void setValue(const size_t& column, const size_t& row, const T& value) {
    if (row < _main->size()) {
      _main->template setValue(column, row, value);
    } else {
      const size_t delta_row = row - _main->size();
      _delta->template setValue(column, delta_row, value);
    }
  }

  template <typename T>
  T getValue(const size_t& column, const size_t& row) const {
    if (row < _main->size()) {
      return _main->template getValue<T>(column, row);
    } else {
      const size_t delta_row = row - _main->size();
      return _delta->template getValue<T>(column, delta_row);
    }
  }

  /**
   * @see AbstractTable
   */
  ValueId getValueId(const size_t column, const size_t row) const;

  /**
   * @see AbstractTable
   */
  void setValueId(const size_t column, const size_t row, const ValueId valueId);

  /**
   * @see AbstractTable
   */
  size_t partitionWidth(const size_t slice) const;

  /**
   * @see AbstractTable
   */
  const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, 
                                                  const size_t row = 0, 
                                                  const table_id_t table_id = 0) const;

  /**
   * @see AbstractTable
   */
  const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, 
                                                         const table_id_t table_id) const;

  /**
   * @see AbstractTable
   */
  void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, 
                       const size_t column, const size_t row = 0, const table_id_t table_id = 0);

  unsigned int partitionCount() const;

  ///////////////////////////////////////////////////////////////////////////////////////
  /// Delta Methods
  std::shared_ptr<delta_table_t> getDelta() const { return _delta; }
  std::shared_ptr<main_table_t> getMain() const { return _main; }
  ///////////////////////////////////////////////////////////////////////////////////////
  /// Disabled Methods
  atable_ptr_t copy() const;

  /**
   * @see AbstractTable
   */
  atable_ptr_t copy_structure(const field_list_t *fields = nullptr, 
                                                        const bool reuse_dict = false, 
                                                        const size_t initial_size = 0, 
                                                        const bool with_containers = true, 
                                                        const bool compressed = false) const {
    STORAGE_NOT_IMPLEMENTED(SimpleStore, copy_structure());
  }

  void merge();
  void mergeWith(std::unique_ptr<TableMerger> merger);
  virtual void print(const size_t limit = (size_t) -1) const;

  virtual void debugStructure(size_t level=0) const;

private:

  /**
   * Create the delta partition based on the structure of the main table
   */
  void createDelta();

  std::unique_ptr<TableMerger> _merger;

};

}}

