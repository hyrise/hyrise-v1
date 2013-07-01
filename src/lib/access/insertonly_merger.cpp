// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/insertonly_merger.h"

#include <set>

#include "storage/RawTable.h"
#include "storage/SimpleStore.h"
#include "storage/meta_storage.h"
#include "access/insertonly_valid.h"

namespace hyrise { namespace insertonly {

/**
 * This class implements the part for merging the dictionaries between
 * the uncompressed delta and the compressed order preserving
 * dictionary. This is done using the following simple
 * algorithm. First a new std::set is created to store all distinct
 * values in a sorted order. Now a new dictionary is created based on
 * this set and a mapping table is build that maps the old values from
 * the old dictionary to the new dictionary. We also need to factor in
 * the valid positions of both main and delta, which can lead to smaller
 * dictionaries due to dropped values.
 */
struct MergeDictFunctor {

  struct result {
    std::vector<value_id_t> mapping;
    std::shared_ptr<AbstractDictionary> dict;
  };

  // Result type value definition
  typedef result value_type;

  const hyrise::storage::c_atable_ptr_t _main;
  const std::shared_ptr<const storage::SimpleStore::delta_table_t>  _delta;
  const storage::pos_list_t& _main_valid;
  const storage::pos_list_t& _delta_valid;
  const field_t& _column;


  MergeDictFunctor(const hyrise::storage::c_atable_ptr_t& m,
                   const std::shared_ptr<const storage::SimpleStore::delta_table_t>& d,
                   const storage::pos_list_t& mv,
                   const storage::pos_list_t& dv,
                   const field_t& c) :
      _main (m),  _delta (d),  _main_valid(mv), _delta_valid(dv), _column(c) {}

  template<typename R>
  result operator()() {
    auto dict = std::dynamic_pointer_cast<OrderPreservingDictionary<R>>(_main->dictionaryAt(_column));
    assert(dict && "Dict shouldn't be NULL, cast failed");
    std::set<R> values;

    // Build unified dictionary from valid delta positons
    for(const auto& position: _delta_valid) {
      values.insert(_delta->getValue<R>(_column, position));
    }

    for(const auto& position: _main_valid) {
      values.insert(_main->getValue<R>(_column, position));
    }

    // Build mapping table for old dictionary
    auto start = values.cbegin();
    auto end = values.cend();

    std::vector<value_id_t> mapping;
    for(size_t i=0; i < dict->size(); ++i) {
      auto val = dict->getValueForValueId(i);
      auto it = std::find(start, end, val);
      if (it != end) {
        mapping.push_back(std::distance(values.cbegin(), it));
        start = ++it; //Move start one beyond where we found this mapping
      } else {
        mapping.push_back(0); // NO MAPPING, should not be mapped
      }
    }

    auto resultDict = std::make_shared<OrderPreservingDictionary<R>>(values.size());
    for(auto e : values)
      resultDict->addValue(e);

    return {mapping, resultDict};
  }

};

/**
 * This class performs the mapping of old uncompressed delta values to
 * new valueIds in the compressed data store.
 */
struct MapValueForValueId {
  typedef void value_type;

  const hyrise::storage::atable_ptr_t& _main;
  field_t _srcCol;
  field_t _dstCol;
  const std::shared_ptr<const storage::SimpleStore::delta_table_t>&  _delta;
  const storage::pos_list_t& _delta_valid;

  MapValueForValueId(const hyrise::storage::atable_ptr_t& m,
                     field_t dst,
                     field_t col,
                     const std::shared_ptr<const storage::SimpleStore::delta_table_t>& de,
                     const storage::pos_list_t& dv) :  _main(m),  _srcCol(col), _dstCol(dst),  _delta(de), _delta_valid(dv) {
  }

  template<typename R>
  value_type operator() () {
    auto _dict = _main->dictionaryAt(_dstCol);
    auto d = std::dynamic_pointer_cast<OrderPreservingDictionary<R>>(_dict);

    size_t start = _main->size() - _delta_valid.size();
    for (size_t position=0, end=_delta_valid.size(); position < end; ++position) {
      size_t srcRow = _delta_valid[position];
      size_t dstRow = start + position;
      _main->setValueId(_dstCol, dstRow, ValueId{d->getValueIdForValue(_delta->getValue<R>(_srcCol, srcRow)), 0});
    }
  }
};


void DiscardingMerger::mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                   hyrise::storage::atable_ptr_t merged_table,
                                   const storage::column_mapping_t &column_mapping,
                                   const uint64_t newSize,
                                   bool useValid,
                                   const std::vector<bool>& valid) {
  
  if (useValid) throw std::runtime_error("SimpleStoreMerger uses a different way to handle valid positions");
  if (input_tables.size() != 2) throw std::runtime_error("SimpleStoreMerger does not support more than two tables");
  auto main = input_tables[0];
  auto delta = std::dynamic_pointer_cast<const RawTable<>>(input_tables[1]);
  assert(main && delta && "main delta need to be valid");

  // Prepare type handling
  auto delta_valid = validPositions(delta, _tx);
  auto main_valid = validPositions(main, _tx);
  storage::type_switch<hyrise_basic_types> ts;

  std::vector<MergeDictFunctor::result> mergedDictionaries;

  // Extract unique values for delta
  for(const auto& kv : column_mapping) {
    auto col = kv.first;
    MergeDictFunctor fun(main, delta, main_valid, delta_valid, col);
    auto result = ts(main->typeOfColumn(col), fun);
    merged_table->setDictionaryAt(result.dict, col);
    mergedDictionaries.push_back(result);
  }

  // Update the values of the new Table
  merged_table->resize(main_valid.size() + delta_valid.size());
  
  for(size_t position=0, end = main_valid.size(); position < end; ++position) {
    size_t row = main_valid[position];
    for(const auto& kv : column_mapping) {
      auto col = kv.first;
      auto dst = kv.second;
      merged_table->setValueId(dst, position, ValueId{mergedDictionaries[col].mapping[main->getValueId(col, row).valueId], 0});
    }
  }

  // Map the values for the values in the uncompressed delta
  for( const auto& kv : column_mapping) {
    auto col = kv.first;
    auto dst = kv.second;
    MapValueForValueId map(merged_table, dst, col, delta, delta_valid);
    ts(merged_table->typeOfColumn(col), map);
  }
}


}}
