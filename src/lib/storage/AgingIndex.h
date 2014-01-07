// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <map>

#include <helper/types.h>
#include <storage/AbstractIndex.h>
#include <storage/AbstractTable.h>

namespace hyrise {
namespace storage {

class AgingIndex : public AbstractIndex {
public:
  AgingIndex(const atable_ptr_t& table);

  virtual ~AgingIndex();

  virtual void shrink();

  bool isHot(query_id_t query, std::vector<value_id_t> values);

private:
  typedef std::map<value_id_t, unsigned> value_id_map_t; // mapping for one row
  typedef std::map<field_t, value_id_map_t> value_id_table_t; // mapping for all relevant fields

  typedef std::vector<bool> hotness_vector_t; // data for one query
  typedef std::map<query_id_t, hotness_vector_t> hotness_map_t; // data for one field
  typedef std::map<field_t, hotness_map_t> hotness_table_t; // data of all relevant fields

  template <typename T>
  static value_id_map_t createVIdMap(const atable_ptr_t& table, field_t column){
    value_id_map_t ret;
    //TODO maybe better function for getting the dictionary
    const auto& dict = checked_pointer_cast<BaseDictionary<T>>(table->dictionaryAt(column));

    auto it = dict->begin();
    const auto& end = dict->end();
    unsigned count = 0;
    while (it != end) {
       ret.insert(std::make_pair(it.getValueId(), count++));
       ++it;
    }

    return ret;
  }


  value_id_table_t _value_id_table;
  hotness_table_t _hotness_table;
};

} } // namespace hyrise::storage


