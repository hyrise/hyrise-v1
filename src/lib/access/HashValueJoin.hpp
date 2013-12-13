// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_HASHVALUEJOIN_HPP_
#define SRC_LIB_ACCESS_HASHVALUEJOIN_HPP_

#include <unordered_map>

#include <memory>

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <access/system/PlanOperation.h>
#include <storage/PointerCalculator.h>

namespace hyrise { namespace access {

template <typename T>
class HashValueJoin : public PlanOperation {
 public:

  typedef std::unordered_multimap<T, pos_t> map_type;

  HashValueJoin() {
    
  }

  virtual ~HashValueJoin() {
    
  }

  /*
   * Expects two inputs
   * @return in the return value, returns the second input and the resulting positions
   */
  void executePlanOperation() {
    if (!producesPositions) {
      throw std::runtime_error("HashValueJoin execute() not supported with generatesPositions == false");
    }

    map_type hash;
    std::vector<pos_t> *build_pos = new std::vector<pos_t>();
    std::vector<pos_t> *probe_pos = new std::vector<pos_t>();

    T value;

    // always use the smaller table as the 'build' table
    storage::c_atable_ptr_t build_table;
    storage::c_atable_ptr_t probe_table;
    field_t build_field, probe_field;
    if (input.getTable(0)->size() <= input.getTable(1)->size()) {
      build_table = input.getTable(0);
      probe_table = input.getTable(1);

      build_field = _field_definition[0];
      probe_field = _field_definition[1];
    } else {
      build_table = input.getTable(1);
      probe_table = input.getTable(0);

      build_field = _field_definition[1];
      probe_field = _field_definition[0];
    }

    // build the hash using the smaller table

    size_t input_size = build_table->size();

    for (pos_t row = 0; row < input_size; row++) {
      value = build_table->getValue<T>(build_field, row);
      hash.insert(typename map_type::value_type(value, row));
    }

    // probe the hash for each row in the bigger table
    input_size = probe_table->size();

    for (size_t row = 0; row < input_size; ++row) {
      value = probe_table->getValue<T>(probe_field, row);

      std::pair<typename map_type::iterator, typename map_type::iterator> pair1 = hash.equal_range(value);

      for (; pair1.first != pair1.second; ++pair1.first) {
        build_pos->push_back(pair1.first->second);
        probe_pos->push_back(row);
      }
    }

    // input.getTable(0) will always be the left part of our output, no matter
    // if it's the build table or not
    std::vector<storage::atable_ptr_t > parts;
    // FIXME: Worst stuff ever
    auto build_pc = std::const_pointer_cast<storage::AbstractTable>(std::dynamic_pointer_cast<const storage::AbstractTable>(storage::PointerCalculator::create(build_table, build_pos)));
    auto probe_pc = std::const_pointer_cast<storage::AbstractTable>(std::dynamic_pointer_cast<const storage::AbstractTable>(storage::PointerCalculator::create(probe_table, probe_pos)));
    if (build_table == input.getTable(0)) {
      parts.push_back(build_pc);
      parts.push_back(probe_pc);
    } else {
      parts.push_back(probe_pc);
      parts.push_back(build_pc);
    }
    // TOOD add TableView
    addResult(std::make_shared<const storage::MutableVerticalTable>(parts));
  }

  const std::string vname() {
    return "HashValueJoin";
  }

};

}}
#endif  // SRC_LIB_ACCESS_HASHVALUEJOIN_HPP_

