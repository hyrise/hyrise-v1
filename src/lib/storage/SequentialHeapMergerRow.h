
#ifndef SRC_LIB_STORAGE_SEQUENTIALHEAPMERGERROW_H_
#define SRC_LIB_STORAGE_SEQUENTIALHEAPMERGERROW_H_

#include <storage/ValueIdMap.hpp>
#include <storage/AbstractTable.h>
#include <storage/AbstractMerger.h>
#include <storage/DictionaryPosition.h>

class SequentialHeapMergerRow : public AbstractMerger {
public:

  virtual void mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                           hyrise::storage::atable_ptr_t merged_table,
                           const hyrise::storage::column_mapping_t &column_mapping,
                           const uint64_t newSize);
  virtual AbstractMerger *copy();

private:
  template <typename T>
  void mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, size_t source_index, 
    size_t column_index, hyrise::storage::atable_ptr_t merged_table,
    std::vector<std::vector<value_id_t> > &m, std::vector<AbstractTable::SharedDictionaryPtr> &d);

  void copyValuesRowWise(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, 
    hyrise::storage::atable_ptr_t merged_table, std::vector<std::vector<std::vector<value_id_t> > > &mappings,
    const hyrise::storage::column_mapping_t &column_mapping);

  void copyValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, size_t column_index, 
    hyrise::storage::atable_ptr_t &merged_table, std::vector<std::vector<value_id_t> > &value_id_mapping);

  template <typename T>
  AbstractTable::SharedDictionaryPtr createNewDict(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, 
    std::vector<AbstractTable::SharedDictionaryPtr > &value_id_maps, std::vector<std::vector<value_id_t> > &value_id_mapping);

};

#endif  // SRC_LIB_STORAGE_SEQUENTIALHEAPMERGERROW_H_
