// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_HISTOGRAM_H_
#define SRC_LIB_ACCESS_HISTOGRAM_H_

#include "PlanOperation.h"

#include <cstdint>
#include <utility>

#include <helper/types.h>
#include <storage/FixedLengthVector.h>
#include <storage/OrderPreservingDictionary.h>
#include <storage/PointerCalculator.h>

namespace hyrise { namespace access {

// Extracts the AV from the table at given column
template<typename Table, typename VectorType=FixedLengthVector<value_id_t> >
inline std::pair<std::shared_ptr<VectorType>, size_t> _getDataVector(Table tab, size_t column=0) {
  const auto& avs = tab->getAttributeVectors(column);
  const auto data = std::dynamic_pointer_cast<VectorType>(avs.at(0).attribute_vector);
  assert(data != nullptr);
  return {data, avs.at(0).attribute_offset};
}

template<typename VectorType=FixedLengthVector<value_id_t> >
inline std::pair<std::shared_ptr<VectorType>, size_t> getDataVector(const hyrise::storage::c_atable_ptr_t& tab, size_t column=0) {
  return _getDataVector<decltype(tab), VectorType>(tab, column);
}



/**
  * This is a Histogram Plan Operation that calculates the number
  * occurences of a single value based on a hash function and a number
  * of significant bits. This Operation is used for the Radix Join
  * implementation
  */
class Histogram : public _PlanOperation {
public:
  Histogram();

  virtual ~Histogram();

  virtual void executePlanOperation();

  template<typename T>
  void executeHistogram();
  
  /**
  * Parses the JSON string to create the plan operation, parameters
  * to the json are:
  * 
  *  - bits: to set the number of used bits for the histogram
  */
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  const std::string vname() { return "Histogram"; }

  inline void setBits(uint32_t b, uint32_t sig=0) {
    _bits = b;
    _significantOffset = sig;
  }

  inline void setBits2(uint32_t b, uint32_t sig=0) {
    _bits2 = b;
    _significantOffset2 = sig;
  }

  inline uint32_t bits() const { return _bits; }
  inline uint32_t significantOffset() const { return _significantOffset; }

  inline void setPart(size_t p) { _part = p; }
  inline void setCount(size_t c) { _count = c; }


  void splitInput(){};
protected:

  static bool is_registered;

  // Create the output table
  std::shared_ptr<Table<>> createOutputTable(size_t size);

  // First Pass
  uint32_t _bits;
  uint32_t _significantOffset;

  //Second pass
  uint32_t _bits2;
  uint32_t _significantOffset2;
  
  size_t _part;
  size_t _count;
};

template<typename T>
void Histogram::executeHistogram() {
  auto tab = getInputTable();
  auto tableSize = getInputTable()->size();
  const auto field = _field_definition[0];

  //Prepare mask
  auto mask = ((1 << bits()) - 1) << significantOffset();

  // Prepare Output Table
  auto result = createOutputTable(1 << bits());
  auto pair = getDataVector(result);

  // Iterate and hash based on the part description
  size_t start=0, stop=tableSize;
  if (_count > 0) {
    start = (tableSize / _count) * _part;
    stop = (_count -1) == _part ? tableSize : (tableSize/_count) * (_part + 1);
  }
  // check if tab is PointerCalculator; if yes, get underlying table and actual rows and columns
  auto p = std::dynamic_pointer_cast<const PointerCalculator>(tab);
  if (p) {
    auto ipair = getDataVector(p->getActualTable());
    const auto& ivec = ipair.first;

    const auto& dict = std::dynamic_pointer_cast<OrderPreservingDictionary<T>>(tab->dictionaryAt(p->getTableColumnForColumn(field)));
    const auto& offset = p->getTableColumnForColumn(field) + ipair.second;

    auto hasher = std::hash<T>();
    for(decltype(tableSize) row = start; row < stop; ++row) {
      //fun.setRow(row);
      auto hash_value  = hasher(dict->getValueForValueId(ivec->get(offset, p->getTableRowForRow(row))));
      pair.first->inc(0, (hash_value & mask) >> significantOffset());
    }
  } else {
    auto ipair = getDataVector(tab);
    const auto& ivec = ipair.first;
    const auto& dict = std::dynamic_pointer_cast<OrderPreservingDictionary<T>>(tab->dictionaryAt(field));
    const auto& offset = field + ipair.second;

    auto hasher = std::hash<T>();
    for(decltype(tableSize) row = start; row < stop; ++row) {
      //fun.setRow(row);
      auto hash_value  = hasher(dict->getValueForValueId(ivec->get(offset, row)));
      pair.first->inc(0, (hash_value & mask) >> significantOffset());
    }
  }
  addResult(result);  
}


class Histogram2ndPass : public Histogram
{

  static bool is_registered;  

public:
  Histogram2ndPass();
  virtual ~Histogram2ndPass();

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  const std::string vname() { return "Histogram2ndPass"; }

};

}}

#endif //SRC_LIB_ACCESS_HISTOGRAM_H_
