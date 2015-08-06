// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_HISTOGRAM_H_
#define SRC_LIB_ACCESS_HISTOGRAM_H_

#include "access/system/ParallelizablePlanOperation.h"

#include "storage/FixedLengthVector.h"
#include "storage/BaseAttributeVector.h"
#include "storage/BaseDictionary.h"
#include "storage/PointerCalculator.h"
#include "storage/Store.h"
#include "helper/types.h"

namespace hyrise {
namespace access {

// Extracts the AV from the table at given column
template <typename VectorType, typename Table>
inline std::pair<std::shared_ptr<VectorType>, size_t> _getDataVector(const Table& tab, const size_t column = 0) {
  const auto& avs = tab->getAttributeVectors(column);
  assert(avs.size() == 1 || (avs.size() == 2 && std::dynamic_pointer_cast<storage::BaseAttributeVector<value_id_t>>(
                                                    avs.at(1).attribute_vector)->size() == 0));
  const auto& data = std::dynamic_pointer_cast<VectorType>(avs.front().attribute_vector);
  assert(data != nullptr);
  return {data, avs.front().attribute_offset};
}

template <typename VectorType = storage::AbstractFixedLengthVector<value_id_t>>
inline std::pair<std::shared_ptr<VectorType>, size_t> getFixedDataVector(const storage::c_atable_ptr_t& tab,
                                                                         const size_t column = 0) {
  return _getDataVector<VectorType>(tab, column);
}

template <typename VectorType = storage::BaseAttributeVector<value_id_t>>
inline std::pair<std::shared_ptr<VectorType>, size_t> getBaseVector(const storage::c_atable_ptr_t& tab,
                                                                    const size_t column) {
  return _getDataVector<VectorType>(tab, column);
}

// Execute the main work of histogram and cluster
template <typename T, typename ResultType = storage::FixedLengthVector<value_id_t>>
void _executeRadixHashing(storage::c_atable_ptr_t sourceTab,
                          size_t field,
                          size_t start,
                          size_t stop,
                          uint32_t bits,
                          uint32_t significantOffset,
                          std::shared_ptr<ResultType> result_av,
                          std::shared_ptr<ResultType> data_hash = nullptr,
                          std::shared_ptr<ResultType> data_pos = nullptr) {
  storage::c_atable_ptr_t tab;
  size_t column;
  const pos_list_t* pc_pos_list;

  if (auto pc = std::dynamic_pointer_cast<const storage::PointerCalculator>(sourceTab)) {
    tab = pc->getActualTable();
    column = pc->getTableColumnForColumn(field);
    pc_pos_list = pc->getPositions();
  } else {
    // output of radix join is MutableVerticalTable of PointerCalculators
    if (auto mvt = std::dynamic_pointer_cast<const storage::MutableVerticalTable>(sourceTab)) {
      auto container = mvt->containerAt(field);
      auto fieldInContainer = mvt->getOffsetInContainer(field);
      if (auto pc = std::dynamic_pointer_cast<const storage::PointerCalculator>(container)) {
        tab = pc->getActualTable();
        column = pc->getTableColumnForColumn(fieldInContainer);
        pc_pos_list = pc->getPositions();
      } else {
        throw std::runtime_error(
            "Radix only supports MutableVerticalTable of PointerCalculators; found other AbstractTable than "
            "PointerCalculator inside MutableVerticalTable.");
      }
    } else {
      // else; we expect a raw table
      tab = sourceTab;
      column = field;
      pc_pos_list = nullptr;
    }
  }

  const auto& store = std::dynamic_pointer_cast<const storage::Store>(tab);
  if (!store) {
    throw std::runtime_error("Could not cast to store!");
  }

  const auto& main = store->getMainTable();
  const auto& delta = store->getDeltaTable();

  std::shared_ptr<storage::BaseAttributeVector<value_id_t>> ivec_main, ivec_delta;
  size_t offset_main, offset_delta;
  std::tie(ivec_main, offset_main) = getBaseVector(main, column);
  std::tie(ivec_delta, offset_delta) = getBaseVector(delta, column);

  size_t main_size = ivec_main->size();

  const auto& main_dict = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(main->dictionaryAt(column));
  const auto& delta_dict = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(delta->dictionaryAt(column));

  auto hasher = std::hash<T>();
  auto mask = ((1 << bits) - 1) << significantOffset;
  size_t hash_value;
  for (size_t row = start; row < stop; ++row) {
    size_t actual_row = pc_pos_list ? pc_pos_list->at(row) : row;
    if (actual_row < main_size) {
      hash_value = hasher(main_dict->getValueForValueId(ivec_main->get(offset_main, actual_row)));
    } else {
      hash_value = hasher(delta_dict->getValueForValueId(ivec_delta->get(offset_delta, actual_row - main_size)));
    }
    // happens for histogram
    auto pos_to_write = result_av->inc(0, (hash_value & mask) >> significantOffset);
    // happens for cluster
    if (data_hash) {
      data_hash->set(0, pos_to_write, hash_value);
    }
    if (data_pos) {
      data_pos->set(0, pos_to_write, row);
    }
  }
}

/// This is a Histogram Plan Operation that calculates the number
/// occurences of a single value based on a hash function and a number
/// of significant bits. This Operation is used for the Radix Join
/// implementation
class Histogram : public ParallelizablePlanOperation {
 public:
  Histogram();

  void executePlanOperation();
  /// Parses the JSON string to create the plan operation, parameters
  /// to the json are:
  ///  - bits: to set the number of used bits for the histogram
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  template <typename T>
  void executeHistogram();
  void setPart(const size_t p);
  void setCount(const size_t c);
  void setBits(const uint32_t b, const uint32_t sig = 0);
  void setBits2(const uint32_t b, const uint32_t sig = 0);
  uint32_t bits() const;
  uint32_t significantOffset() const;

 protected:
  std::shared_ptr<storage::Table> createOutputTable(const size_t size) const;

  uint32_t _bits;
  uint32_t _significantOffset;
  uint32_t _bits2;
  uint32_t _significantOffset2;
  size_t _part;
  size_t _count;
};

template <typename T>
void Histogram::executeHistogram() {
  const auto& tab = getInputTable();
  const auto tableSize = getInputTable()->size();
  const auto field = _field_definition[0];

  // Prepare Output Table
  auto result = createOutputTable(1 << bits());
  auto result_av = getFixedDataVector(result).first;

  // Iterate and hash based on the part description
  size_t start = 0, stop = tableSize;
  if (_count > 0) {
    start = (tableSize / _count) * _part;
    stop = (_count - 1) == _part ? tableSize : (tableSize / _count) * (_part + 1);
  }

  _executeRadixHashing<T>(tab, field, start, stop, bits(), significantOffset(), result_av);

  addResult(result);
}

class Histogram2ndPass : public Histogram {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
};
}
}

#endif  // SRC_LIB_ACCESS_HISTOGRAM_H_
