// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <storage/AbstractStatistic.h>

namespace hyrise {
namespace access {

class MapStatistic : public storage::AbstractStatistic {
// TODO implement from former AgingIndex implementation
};

} } // namespace hyrise::access

/*#include <unordered_map>

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

  void addForQuery(access::query_t query, const std::vector<field_t>& fields);

  struct param_t {
    field_t field;
    value_id_t vid;
  };
  bool isHot(access::query_t query, std::vector<param_t> params);

  atable_ptr_t table();

  typedef std::unordered_map<value_id_t, unsigned> value_id_map_t; // mapping for one row
  typedef std::unordered_map<field_t, value_id_map_t> value_id_table_t; // mapping for all relevant fields

  typedef std::vector<bool> hotness_vector_t; // data for one query
  typedef std::unordered_map<access::query_t, hotness_vector_t> hotness_map_t; // data for one field
  typedef std::unordered_map<field_t, hotness_map_t> hotness_table_t; // data of all relevant fields

private:
  void add(access::query_t query, field_t field, value_id_t value, bool hot);
  void updateValueIdMapping();

  hotness_map_t& findOrCreate(field_t field);
  hotness_vector_t& findOrCreate(field_t field, access::query_t query);

  value_id_table_t _valueIdTable;
  hotness_table_t _hotnessTable;

  const std::weak_ptr<storage::AbstractTable> _table;
};

} } // namespace hyrise::storage

*/
