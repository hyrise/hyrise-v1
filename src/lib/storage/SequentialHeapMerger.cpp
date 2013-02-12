// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/SequentialHeapMerger.h>

#include <queue>

void SequentialHeapMerger::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                       hyrise::storage::atable_ptr_t merged_table,
                                       const hyrise::storage::column_mapping_t &column_mapping,
                                       const uint64_t newSize) {

  std::vector<value_id_mapping_t> mappingPerAtrtibute(input_tables[0]->columnCount());

for (const auto & kv: column_mapping) {
    const auto &source = kv.first;
    const auto &destination = kv.second;
    switch (merged_table->metadataAt(destination)->getType()) {
      case IntegerType:
        mergeValues<hyrise_int_t>(input_tables, source, merged_table, destination, mappingPerAtrtibute[source]);
        break;

      case FloatType:
        mergeValues<hyrise_float_t>(input_tables, source, merged_table, destination, mappingPerAtrtibute[source]);
        break;

      case StringType:
        mergeValues<hyrise_string_t>(input_tables, source, merged_table, destination, mappingPerAtrtibute[source]);
        break;

      default:
        break;
    }
  }

  merged_table->resize(newSize);

  // Only after the dictionaries are merged copy the values
for (const auto & kv: column_mapping) {
    const auto &source = kv.first;
    const auto &destination = kv.second;
    // copy the actual values and apply mapping


    copyValues(input_tables, source, merged_table, destination, mappingPerAtrtibute[source]);
  }
}

template <typename T>
void SequentialHeapMerger::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                       size_t source_column_index,
                                       hyrise::storage::atable_ptr_t merged_table,
                                       size_t destination_column_index,
                                       value_id_mapping_t &value_id_mapping) {

  std::vector<hyrise::storage::dict_ptr_t > value_id_maps;
  hyrise::storage::dict_ptr_t new_dict;

  // shortcut for dicts
  value_id_maps.reserve(input_tables.size());

  for (size_t table = 0; table < input_tables.size(); table++) {
    if ((merged_table->metadataAt(destination_column_index)->getType() !=
         input_tables[table]->metadataAt(source_column_index)->getType())) {
      throw std::runtime_error("Dictionary types don't match");
    }
    auto dict = std::dynamic_pointer_cast<BaseDictionary<T>>(input_tables[table]->dictionaryAt(source_column_index));
    value_id_maps.push_back(dict);
  }

  // Create new BaseDictionary - shrink when merge finished?
  new_dict = createNewDict<T>(input_tables, value_id_maps, value_id_mapping);

  // set new value id map for column
  merged_table->setDictionaryAt(new_dict, destination_column_index);
}

template <typename T>
hyrise::storage::dict_ptr_t SequentialHeapMerger::createNewDict(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, std::vector<hyrise::storage::dict_ptr_t > &value_id_maps, std::vector<std::vector<value_id_t> > &value_id_mapping) {
  hyrise::storage::dict_ptr_t new_dict;
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

  new_dict->shrink();
  return new_dict;
}

void SequentialHeapMerger::copyValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                      size_t source_column_index,
                                      hyrise::storage::atable_ptr_t &merged_table,
                                      size_t destination_column_index,
                                      std::vector<std::vector<value_id_t> > &value_id_mapping) {
  ValueId value_id;

  // copy all value ids to the new doc vector
  // and apply value id mapping
  size_t merged_table_row = 0;

  for (size_t table = 0; table < input_tables.size(); table++) {
    for (size_t row = 0; row < input_tables[table]->size(); row++) {
      value_id.valueId = input_tables[table]->getValueId(source_column_index, row).valueId;
      value_id.valueId = value_id_mapping[table][value_id.valueId]; // translate value id to new dict
      merged_table->setValueId(destination_column_index, merged_table_row, value_id);
      merged_table_row++;
    }
  }

}

AbstractMerger *SequentialHeapMerger::copy() {
  return new SequentialHeapMerger();
}
