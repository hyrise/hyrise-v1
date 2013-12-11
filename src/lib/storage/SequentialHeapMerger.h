// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <storage/ValueIdMap.hpp>
#include <storage/AbstractTable.h>
#include <storage/AbstractMerger.h>

namespace hyrise {
namespace storage {

class SequentialHeapMerger : public AbstractMerger {
public:

  virtual void mergeValues(const std::vector<c_atable_ptr_t > &input_tables,
                           atable_ptr_t merged_table,
                           const column_mapping_t &column_mapping,
                           const uint64_t newSize,
                           bool useValid = false,
                           const std::vector<bool>& valid = std::vector<bool>());
  virtual AbstractMerger *copy();

private:

  typedef std::vector<std::vector<value_id_t> > value_id_mapping_t;

  template <typename T>
  void mergeValues(const std::vector<c_atable_ptr_t > &input_tables,
                   size_t source_column_index,
                   atable_ptr_t  merged_table,
                   size_t destination_column,
                   value_id_mapping_t &mapping,
                   bool useValid,
                   const std::vector<bool>& valid);


  void copyValues(const std::vector<c_atable_ptr_t > &input_tables,
                  size_t source_column_index,
                  atable_ptr_t  &merged_table,
                  size_t destination_column_index,
                  std::vector<std::vector<value_id_t> > &value_id_mapping,
                  bool useValid,
                  const std::vector<bool>& valid);

  template <typename T>
  AbstractTable::SharedDictionaryPtr createNewDict(const std::vector<c_atable_ptr_t > &input_tables,
      std::vector<AbstractTable::SharedDictionaryPtr > &value_id_maps,
      std::vector<std::vector<value_id_t> > &value_id_mapping,
      size_t column_index,
      bool useValid,
      const std::vector<bool>& valid);

};

} } // namespace hyrise::storage

