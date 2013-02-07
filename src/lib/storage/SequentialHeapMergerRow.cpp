// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/SequentialHeapMergerRow.h"

#include <queue>

#include "storage/storage_types.h"

void SequentialHeapMergerRow::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, hyrise::storage::atable_ptr_t merged_table, const hyrise::storage::column_mapping_t &mapping,
    const uint64_t newSize) {
  // First merge all the dictionaries
  typedef std::vector<AbstractTable::SharedDictionaryPtr> dict_list_t;
  typedef std::vector<std::vector<value_id_t> > value_id_mapping_t;

  // Preallocate the list for the mappings per attribute
  std::vector<dict_list_t> dictMappingPerAttribute(input_tables[0]->columnCount());
  std::vector<value_id_mapping_t> mappingPerAtrtibute(input_tables[0]->columnCount());


  for (size_t column_index = 0; column_index < input_tables[0]->columnCount(); column_index++) {
    unsigned dest = mapping.find(column_index)->second;
    switch (merged_table->metadataAt(dest)->getType()) {
      case IntegerType:
        mergeValues<hyrise_int_t>(input_tables, column_index, dest, merged_table, mappingPerAtrtibute[dest], dictMappingPerAttribute[dest]);
        break;

      case FloatType:
        mergeValues<hyrise_float_t>(input_tables, column_index, dest, merged_table, mappingPerAtrtibute[dest], dictMappingPerAttribute[dest]);
        break;

      case StringType:
        mergeValues<hyrise_string_t>(input_tables, column_index, dest, merged_table, mappingPerAtrtibute[dest], dictMappingPerAttribute[dest]);
        break;

      default:
        break;
    }
  }

  // Reverse the mapping
  hyrise::storage::column_mapping_t inverted;
  for (const auto & kv : mapping)
    inverted[kv.second] = kv.first;

  merged_table->resize(newSize);
  // Now perform an container wise update of the tables
  copyValuesRowWise(input_tables, merged_table, mappingPerAtrtibute, inverted);
}

template <typename T>
void SequentialHeapMergerRow::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, size_t source_index, size_t column_index, hyrise::storage::atable_ptr_t merged_table, std::vector<std::vector<value_id_t> > &value_id_mapping, std::vector<AbstractTable::SharedDictionaryPtr > &value_id_maps) {
  AbstractTable::SharedDictionaryPtr new_dict;

  // shortcut for dicts
  value_id_maps.reserve(input_tables.size());

  for (size_t table = 0; table < input_tables.size(); table++) {
    auto dict = std::dynamic_pointer_cast<BaseDictionary<T>>(input_tables[table]->dictionaryAt(source_index));
    //assert(dict->isOrdered());
    value_id_maps.push_back(dict);
  }

  // Create new BaseDictionary - shrink when merge finished?
  new_dict = createNewDict<T>(input_tables, value_id_maps, value_id_mapping);

  // set new value id map for column
  merged_table->setDictionaryAt(new_dict, column_index);
}

template <typename T>
AbstractTable::SharedDictionaryPtr SequentialHeapMergerRow::createNewDict(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, std::vector<AbstractTable::SharedDictionaryPtr > &value_id_maps, std::vector<std::vector<value_id_t> > &value_id_mapping) {
  AbstractTable::SharedDictionaryPtr new_dict;
  std::priority_queue<DictionaryPosition<T> > p_queue;
  size_t new_dict_max_size = 0;
  T last_value;

  // init
  value_id_mapping.resize(input_tables.size());

  for (size_t table = 0; table < input_tables.size(); table++) {
    // reserve space for value id mapping
    value_id_mapping[table].resize(value_id_maps[table]->size());

    // insert initial value in queue
    if (value_id_maps[table]->size() > 0) {
      p_queue.push(DictionaryPosition<T>(table, std::dynamic_pointer_cast<BaseDictionary<T>>(value_id_maps[table])->begin()));
    }

    new_dict_max_size += value_id_maps[table]->size();
  }

  // Create new BaseDictionary - shrink when merge finished?
  new_dict = std::make_shared<OrderPreservingDictionary<T>>(new_dict_max_size);

  if (!p_queue.empty()) {
    DictionaryPosition<T> dict_pos = p_queue.top();
    p_queue.pop();
    std::dynamic_pointer_cast<BaseDictionary<T>>(new_dict)->addValue(dict_pos.getValue());
    last_value = dict_pos.getValue();
    value_id_mapping[dict_pos.index][dict_pos.it.getValueId()] = new_dict->size() - 1;
    dict_pos.it++;

    if (std::dynamic_pointer_cast<BaseDictionary<T>>(value_id_maps[dict_pos.index])->end() != dict_pos.it) {
      p_queue.push(dict_pos);
    }
  }

  while (!p_queue.empty()) {
    DictionaryPosition<T> dict_pos = p_queue.top();
    p_queue.pop();

    if (dict_pos.getValue() != last_value) {
      std::dynamic_pointer_cast<BaseDictionary<T>>(new_dict)->addValue(dict_pos.getValue());
      last_value = dict_pos.getValue();
    }

    value_id_mapping[dict_pos.index][dict_pos.it.getValueId()] = new_dict->size() - 1;
    dict_pos.it++;

    if (std::dynamic_pointer_cast<BaseDictionary<T>>(value_id_maps[dict_pos.index])->end() != dict_pos.it) {
      p_queue.push(dict_pos);
    }
  }

  //      #ifndef NDEBUG
  //      float queue_size = (float)queue_sum / (float)loop_count;
  //      std::cout << loop_count << ", " << add_count << ", " << next_count << ", " << queue_size << ", " << new_dict->size() << std::endl;
  //      #endif

  return new_dict;
}

void SequentialHeapMergerRow::copyValuesRowWise(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, hyrise::storage::atable_ptr_t merged_table, std::vector<std::vector<std::vector<value_id_t> > > &mappings, const hyrise::storage::column_mapping_t &field_mapping) {
  value_id_t v;

  // Iterate over all slices and copy the data rowvise
  size_t offset = 0;
  for (size_t s = 0; s < merged_table->sliceCount(); ++s) {
    size_t numcols = offset + merged_table->getSliceWidth(s) / sizeof(value_id_t);
    size_t mergedRow = 0;

    for (size_t table = 0; table < input_tables.size(); ++table) {
      for (size_t row = 0; row < input_tables[table]->size(); ++row) {
        // Get meta data
        for (size_t col = offset; col < numcols; ++col) {

          auto mapped = field_mapping.find(col)->second;

          // translate the fields if the order has changed map source to new dest field
          v = input_tables[table]->getValueId(mapped, row).valueId;
          ValueId value_id(mappings[col][table][v], 0);
          merged_table->setValueId(col, mergedRow, value_id);
        }
        ++mergedRow;
      }
    }

    // Update the mapping;
    offset = numcols;
  }

}


void SequentialHeapMergerRow::copyValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, 
  size_t column_index, hyrise::storage::atable_ptr_t &merged_table, std::vector<std::vector<value_id_t> > &value_id_mapping) {
  ValueId value_id;

  // copy all value ids to the new doc vector
  // and apply value id mapping
  size_t merged_table_row = 0;

  for (size_t table = 0; table < input_tables.size(); table++) {
    for (size_t row = 0; row < input_tables[table]->size(); row++) {
      value_id.valueId = input_tables[table]->getValueId(column_index, row).valueId;
      value_id.valueId = value_id_mapping[table][value_id.valueId]; // translate value id to new dict
      merged_table->setValueId(column_index, merged_table_row, value_id);
      merged_table_row++;
    }
  }

}

AbstractMerger *SequentialHeapMergerRow::copy() {
  return new SequentialHeapMergerRow();
}
