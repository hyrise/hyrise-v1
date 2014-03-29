// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/CompoundIndexRangeScan.h"

#include "access/system/BasicParser.h"
#include "access/json_converters.h"
#include "access/system/QueryParser.h"
#include "io/StorageManager.h"
#include "helper/checked_cast.h"
#include "storage/storage_types.h"

#include <algorithm>
#include <iostream>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<CompoundIndexRangeScan>("CompoundIndexRangeScan");
}

CompoundIndexRangeScan::CompoundIndexRangeScan() {
  _predicates_added_main = 0;
  _predicates_added_delta = 0;
  _min_predicates_added_main = 0;
  _max_predicates_added_main = 0;
  _validate = 0;
}

CompoundIndexRangeScan::~CompoundIndexRangeScan() {}



void CompoundIndexRangeScan::executePlanOperation() {
  if (!_main_index_name.empty())
    setMainIndex(_main_index_name);
  if (!_delta_index_name.empty())
    setDeltaIndex(_delta_index_name);


  pos_list_t* result = new pos_list_t;
  assert(_validate);  // for now this is only to be used with a TX context..
  assert(!_unique_index);  // If a range is requested, we do not exit after the first found ..

  if (!_main_index) {
    throw std::runtime_error("Missing Main Index");
  }
  if (!_delta_index) {
    throw std::runtime_error("Missing Delta Index");
  }
  // The index performs a validation of results directly, minimizing the overhead for copying large, invalid positions
  // around.

  // Note: A large part of the index-lookup of the main is already performed during the predicate evaluation, when all
  // predicate-values are checked against the Main dictioanries.
  // therefore we seperatly create the predicates for the delta first, to save that work in the unqiue index-case


  // Refactoring Discussion:
  // The best solution might be a Planop "IndexLookup" for Complete-Key Lookups
  // and a seperate Planop IndexScan for partial keys.
  //

  auto c_store = checked_pointer_cast<const storage::Store>(input.getTables()[0]);
  pos_list_t validated_delta_result;

  if (!_unique_index and !_json_predicates.empty())
    parseJsonPredicates(/*set_for_main=*/true, /*set_for_delta=*/true);

  if (_delta_index) {
    if (_unique_index and !_json_predicates.empty())
      parseJsonPredicates(/*set_for_main=*/false, /*set_for_delta=*/true);

    // partial search
    auto s_key = _value_key_builder_min.get();
    auto up_key = _value_key_builder_max.get_upperbound();
    assert(s_key <= up_key);
    auto iterator_pair = _delta_index->getIteratorsForKeyBetween(s_key, up_key);
    for (auto dp = iterator_pair.first; dp != iterator_pair.second; ++dp) {
      if (c_store->isVisibleForTransaction(dp->second, _txContext.lastCid, _txContext.tid)) {
        validated_delta_result.push_back(dp->second);
      }
    }
  }

  if (_main_index) {
    if (_unique_index and !_json_predicates.empty())
      parseJsonPredicates(/*set_for_main=*/true, /*set_for_delta=*/false);

    compound_valueid_key_t search_key = _valueid_key_builder_min.get();
    compound_valueid_key_t search_key_up = _valueid_key_builder_max.get();

    // hack to handle predicates which consist of only minPredicate
    if (search_key_up == 0 && search_key != 0)
      search_key_up = std::numeric_limits<compound_valueid_key_t>::max();

    compound_valueid_key_t valid_bits = _valueid_key_builder_max.getValidBitMaskForCurrentKey();
    compound_valueid_key_t upper_bound = search_key_up | (~valid_bits);
    // std::cout << "Keys " << search_key << "  " << upper_bound << std::endl;
    storage::PositionRange main_result = _main_index->getPositionsForKeyBetween(search_key, upper_bound);
    result->resize(main_result.size());
    std::copy(main_result.cbegin(), main_result.cend(), result->begin());

    // note: can this break if the largest value id is a power of two?

    c_store->validatePositions(*result, _txContext.lastCid, _txContext.tid);
  }


  size_t real_result_size = result->size();  // we need to resize the result list below
  result->resize(result->size() + validated_delta_result.size());
  std::copy(validated_delta_result.cbegin(), validated_delta_result.cend(), result->begin() + real_result_size);



  addResult(storage::PointerCalculator::create(input.getTables()[0], result));
}


void CompoundIndexRangeScan::setMainIndexDeferred(const std::string& index_name) {
  // we need this method because at time of JSON parsing, the index might not be available yet
  _main_index_name = index_name;
}

void CompoundIndexRangeScan::setDeltaIndexDeferred(const std::string& index_name) { _delta_index_name = index_name; }

void CompoundIndexRangeScan::setMainIndex(const std::string& index_name) {
  if (_predicates_added_main > 0)
    throw std::runtime_error("You cannot set the main index after a predicate has been added");
  _main_index = checked_pointer_cast<storage::GroupkeyIndex<compound_valueid_key_t>>(
      io::StorageManager::getInstance()->getInvertedIndex(index_name, true));
}

void CompoundIndexRangeScan::setDeltaIndex(const std::string& index_name) {
  if (_predicates_added_delta > 0)
    throw std::runtime_error("You cannot set the delta index after a predicate has been added");
  _delta_index = checked_pointer_cast<storage::DeltaIndex<compound_value_key_t>>(
      io::StorageManager::getInstance()->getInvertedIndex(index_name, true));
}

const std::string CompoundIndexRangeScan::vname() { return "CompoundIndexRangeScan"; }

std::shared_ptr<PlanOperation> CompoundIndexRangeScan::parse(const Json::Value& data) {
  std::shared_ptr<CompoundIndexRangeScan> idx_scan = BasicParser<CompoundIndexRangeScan>::parse(data);

  if (data.isMember("mainindex"))
    idx_scan->setMainIndexDeferred(data["mainindex"].asString());
  if (data.isMember("deltaindex"))
    idx_scan->setDeltaIndexDeferred(data["deltaindex"].asString());

  if (!data.isMember("predicates"))
    throw std::runtime_error("No predicates passed");
  idx_scan->_json_predicates = data["predicates"];

  return idx_scan;
}

void CompoundIndexRangeScan::parseJsonPredicates(bool set_for_main, bool set_for_delta) {
  for (auto&& predicate : _json_predicates) {
    if (predicate.size() != 2)
      throw std::runtime_error("Expecting [column, value] for predicate");
    storage::type_switch<hyrise_basic_types> ts;
    AddPredicateFunctor functor(this, predicate[0].asInt(), predicate[1], set_for_main, set_for_delta);
    // get Table
    ts(this->input.getTables()[0]->typeOfColumn(predicate[0].asInt()), functor);
  }
}
}
}
