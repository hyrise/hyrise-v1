// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/system/PlanOperation.h"
#include "storage/storage_types_helper.h"
#include "storage/CompoundValueKeyBuilder.h"
#include "storage/CompoundValueIdKeyBuilder.h"
#include "storage/Store.h"
#include "storage/GroupkeyIndex.h"
#include "storage/DeltaIndex.h"
#include "storage/OrderPreservingDictionary.h"
#include "access/json_converters.h"
#include "helper/checked_cast.h"

namespace hyrise {
namespace access {

class CompoundIndexScan : public PlanOperation {
 public:
  CompoundIndexScan();
  virtual ~CompoundIndexScan();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  void setMainIndex(const std::string& index_name);
  void setDeltaIndex(const std::string& index_name);

  template <typename T>
  void addPredicate(std::string column, T value) {
    addPredicateToDelta<T>(input.getTables()[0]->numberOfColumn(column), value);
    addPredicateToMain<T>(input.getTables()[0]->numberOfColumn(column), value);
  }


  template <typename T>
  void addPredicate(field_t column, T value) {
    addPredicateToMain<T>(column, value);
    addPredicateToDelta<T>(column, value);
  }

  template <typename T>
  void addPredicateToMain(field_t column, T value) {
    if (input.getTables().size() != 1)
      throw std::runtime_error("Please add a table first");

    if (!_main_index && !_delta_index) {
      auto c_store = std::dynamic_pointer_cast<const storage::Store>(input.getTable(0));
      if (!c_store) {
        throw std::runtime_error("Please specify at least one index name or create a store index via CreateStoreIndex");
      }

      throw std::runtime_error("Please add at least one index first");
    }

    if (_main_index) {
      if (_main_index->getColumns().size() == _predicates_added_main)
        throw std::runtime_error("Wrong number of columns for main index specified");
      if (_main_index->getColumns()[_predicates_added_main] != column) {
        throw std::runtime_error(
            "Column mismatch - please specify columns for main index in the order in which they were created");
      }

      auto deb = input.getTables()[0]->dictionaryAt(column);
      value_id_t value_id;
      size_t size;

      auto main_dict =
          checked_pointer_cast<storage::OrderPreservingDictionary<T>>(input.getTables()[0]->dictionaryAt(column));
      value_id = main_dict->findValueIdForValue(value);
      size = main_dict->size();
      // if this checked cast fails, check if you passed the right T (e.g., hyrise_int_t instead of int)

      if (value_id == std::numeric_limits<value_id_t>::max()) {
        // if we can't find a value id for this value, it won't be in the main
        _main_index = nullptr;
      } else {
        _valueid_key_builder.add(value_id, size);
      }
    }


    _predicates_added_main++;
  }

  template <typename T>
  void addPredicateToDelta(field_t column, T value) {
    if (input.getTables().size() != 1)
      throw std::runtime_error("Please add a table first");

    if (!_main_index && !_delta_index) {
      auto c_store = std::dynamic_pointer_cast<const storage::Store>(input.getTable(0));
      if (!c_store) {
        throw std::runtime_error("Please specify at least one index name or create a store index via CreateStoreIndex");
      }
      throw std::runtime_error("Please add at least one index first");
    }


    if (_delta_index) {
      // the delta index currently does not know that it's on multiple columns
      // we could check this via the Store, but for now we assume that the check on the main is sufficient

      _value_key_builder.add(value);
    }

    _predicates_added_delta++;
  }

  const std::string vname();

  void setValidation(bool validate) { _validate = validate; }
  void setUniqueIndex(bool unique_index) { _unique_index = unique_index; }

 private:
  std::shared_ptr<storage::GroupkeyIndex<compound_valueid_key_t>> _main_index;
  std::shared_ptr<storage::DeltaIndex<compound_value_key_t>> _delta_index;
  std::string _main_index_name, _delta_index_name;
  Json::Value _json_predicates;
  bool _validate = false;
  bool _unique_index = false;

  unsigned int _predicates_added_delta;
  unsigned int _predicates_added_main;
  storage::CompoundValueIdKeyBuilder _valueid_key_builder;
  storage::CompoundValueKeyBuilder _value_key_builder;

  void setMainIndexDeferred(const std::string& index_name);
  void setDeltaIndexDeferred(const std::string& index_name);
  void parseJsonPredicates(bool set_for_main = true, bool set_for_delta = true);

  class AddPredicateFunctor {
    CompoundIndexScan* _idx_scan;
    field_t _column;
    Json::Value _value;
    bool _main;
    bool _delta;

   public:
    typedef bool value_type;
    AddPredicateFunctor(CompoundIndexScan* idx_scan, field_t column, Json::Value value, bool main = 1, bool delta = 1)
        : _idx_scan(idx_scan), _column(column), _value(value), _main(main), _delta(delta) {}

    template <typename ValueType>
    value_type operator()() {
      ValueType real_value = json_converter::convert<ValueType>(_value);
      if (_main)
        _idx_scan->addPredicateToDelta(_column, real_value);
      if (_delta)
        _idx_scan->addPredicateToMain(_column, real_value);
      return true;
    }
  };
};
}
}
