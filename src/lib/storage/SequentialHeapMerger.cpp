// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/SequentialHeapMerger.h"

#include <queue>

#include "helper/vector_helpers.h"
#include "storage/DictionaryIterator.h"
#include "storage/ColumnMetadata.h"


void SequentialHeapMerger::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                       hyrise::storage::atable_ptr_t merged_table,
                                       const hyrise::storage::column_mapping_t &column_mapping,
                                       const uint64_t newSize,
                                       bool useValid,
                                       const std::vector<bool>& valid) {

  //if (input_tables.size () != 2)
  //  throw std::runtime_error("Merging more than 2 tables is not supported with this merger...");

  std::vector<value_id_mapping_t> mappingPerAtrtibute(input_tables[0]->columnCount());

  for (const auto & kv: column_mapping) {
    const auto &source = kv.first;
    const auto &destination = kv.second;
    switch (merged_table->metadataAt(destination)->getType()) {
      case IntegerType:
        mergeValues<hyrise_int_t>(input_tables, source, merged_table, destination, mappingPerAtrtibute[source], useValid, valid);
        break;

      case FloatType:
        mergeValues<hyrise_float_t>(input_tables, source, merged_table, destination, mappingPerAtrtibute[source], useValid, valid);
        break;

      case StringType:
        mergeValues<hyrise_string_t>(input_tables, source, merged_table, destination, mappingPerAtrtibute[source], useValid, valid);
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
    copyValues(input_tables, source, merged_table, destination, mappingPerAtrtibute[source], useValid, valid);
  }
}

template <typename T>
void SequentialHeapMerger::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                       size_t source_column_index,
                                       hyrise::storage::atable_ptr_t merged_table,
                                       size_t destination_column_index,
                                       value_id_mapping_t &value_id_mapping,
                                       bool useValid,
                                       const std::vector<bool>& valid) {

  std::vector<AbstractTable::SharedDictionaryPtr > value_id_maps;
  AbstractTable::SharedDictionaryPtr new_dict;

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
  new_dict = createNewDict<T>(input_tables, value_id_maps, value_id_mapping, source_column_index, useValid, valid);
  // set new value id map for column
  merged_table->setDictionaryAt(new_dict, destination_column_index);
}

template<typename T>
struct DictMergeHelper {

  size_t tab_index;
  std::vector<bool> ref;
  DictionaryIterator<T> it;
  DictionaryIterator<T> end;

  bool operator<(const DictMergeHelper &other) const {
    return *it > *(other.it);
  }

  void next() {
    it++;
  }

  bool valid() {
    const auto& vid = it.getValueId();
    return ref[vid];
  }

  bool done() {
    return it == end;
  }

  void print() {
    std::cout << tab_index << " " << *it << std::boolalpha << valid() << std::endl;
  }

};

template <typename T>
AbstractTable::SharedDictionaryPtr SequentialHeapMerger::createNewDict(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables, 
                                                                        std::vector<AbstractTable::SharedDictionaryPtr > &value_id_maps, 
                                                                        std::vector<std::vector<value_id_t> > &value_id_mapping, 
                                                                        size_t column_index,
                                                                        bool useValid,
                                                                        const std::vector<bool>& valid) {
  // Heap Queue
  std::priority_queue<DictMergeHelper<T>> queue;

  value_id_mapping.resize(input_tables.size());

  size_t part_counter = 0;
  for(size_t p=0, stop=input_tables.size(); p < stop; ++p) {
    
    auto dict = std::dynamic_pointer_cast<BaseDictionary<T>>(value_id_maps[p]);
    value_id_mapping[p].resize(value_id_maps[p]->size());

    DictMergeHelper<T> helper;
    helper.it = dict->begin();
    helper.end = dict->end();
    helper.tab_index = p;

    if (useValid) {
      helper.ref.resize(dict->size(), false);
      for(size_t i=0, tabs = input_tables[p]->size(); i < tabs; ++i) {
        if (valid[part_counter + i]) {
             helper.ref[input_tables[p]->getValueId(column_index, i).valueId] = true;
        }
      }
    } else {
      helper.ref.resize(dict->size(), true);
    }

    if (!helper.done()) {
      queue.push(helper);
    }

    // Update the row counter
    part_counter += input_tables[p]->size();
  }

  // Create new Dict
  bool first = true;
  bool assigned = false;

  T last_value;
  auto new_dict = std::make_shared<OrderPreservingDictionary<T>>( hyrise::functional::sum(value_id_mapping, 0ul, [](std::vector<value_id_t>& v){ return v.size(); }));
  while(!queue.empty()) {

    auto element = queue.top();

    T curr_val = *(element.it);
    queue.pop();

    if (first || !assigned || (last_value != curr_val)) {
      if (element.valid()) {
        new_dict->addValue(curr_val);
      }
    }

    if (element.valid()) {
      value_id_mapping[element.tab_index][element.it.getValueId()] = new_dict->size() - 1;
      last_value = curr_val;
      assigned = true;
    }

    element.next();
    if (!element.done()) {
      queue.push(element);
    }
    first = false;
  }

  new_dict->shrink();
  return new_dict;
}

void SequentialHeapMerger::copyValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                      size_t source_column_index,
                                      hyrise::storage::atable_ptr_t &merged_table,
                                      size_t destination_column_index,
                                      std::vector<std::vector<value_id_t> > &value_id_mapping,
                                      bool useValid,
                                      const std::vector<bool>& valid) {
  ValueId value_id;

  // copy all value ids to the new doc vector
  // and apply value id mapping
  size_t merged_table_row = 0;

  size_t part_counter = 0;
  for (size_t table = 0; table < input_tables.size(); table++) {
    for (size_t row = 0; row < input_tables[table]->size(); row++) {
      if (!useValid || (useValid && valid[part_counter + row])) {
        value_id.valueId = input_tables[table]->getValueId(source_column_index, row).valueId;
        value_id.valueId = value_id_mapping[table][value_id.valueId]; // translate value id to new dict
        merged_table->setValueId(destination_column_index, merged_table_row, value_id);
        merged_table_row++;
      }
    }
    part_counter += input_tables[table]->size();
  }

}

AbstractMerger *SequentialHeapMerger::copy() {
  return new SequentialHeapMerger();
}
