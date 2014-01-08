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

  bool isHot(access::query_id_t query, field_t field, value_id_t vid);

  typedef std::map<value_id_t, unsigned> value_id_map_t; // mapping for one row
  typedef std::map<field_t, value_id_map_t> value_id_table_t; // mapping for all relevant fields

  typedef std::vector<bool> hotness_vector_t; // data for one query
  typedef std::map<access::query_id_t, hotness_vector_t> hotness_map_t; // data for one field
  typedef std::map<field_t, hotness_map_t> hotness_table_t; // data of all relevant fields

private:
  value_id_table_t _valueIdTable;
  hotness_table_t _hotnessTable;

  const atable_ptr_t _table;
};

} } // namespace hyrise::storage


