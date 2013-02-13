// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_HISTOGRAM_H_
#define SRC_LIB_ACCESS_HISTOGRAM_H_

#include "PlanOperation.h"

#include <cstdint>
#include <utility>

#include <helper/types.h>
#include <storage/FixedLengthVector.h>
#include <storage/OrderPreservingDictionary.h>

namespace hyrise { namespace access {

// Extracts the AV from the table at given column
template<typename Table, typename VectorType=FixedLengthVector<value_id_t> >
inline std::pair<std::shared_ptr<VectorType>, size_t> _getDataVector(Table tab, size_t column=0) {
  const auto& avs = tab->getAttributeVectors(column);
  const auto data = std::dynamic_pointer_cast<VectorType>(avs.at(0).attribute_vector);
  assert(data != nullptr);
  return {data, avs.at(0).attribute_offset};
}

// template<typename VectorType=FixedLengthVector<value_id_t> >
// inline std::pair<std::shared_ptr<VectorType>, size_t> getDataVector(hyrise::storage::atable_ptr_t tab, size_t column=0) {
//   return _getDataVector<decltype(tab), VectorType>(tab, column);
// }

template<typename VectorType=FixedLengthVector<value_id_t> >
inline std::pair<std::shared_ptr<VectorType>, size_t> getDataVector(const hyrise::storage::c_atable_ptr_t& tab, size_t column=0) {
  return _getDataVector<decltype(tab), VectorType>(tab, column);
}

/*
* Multi fast join hash_value calculator
*/
template<typename T>
struct radix_hash_value {

  typedef T value_type;
  std::shared_ptr<FixedLengthVector<value_id_t>> vector;

  // WARNING: if the input data is a store, this can be tricky if there is data in the delta
  const std::shared_ptr<AbstractDictionary>& dict;
  field_t f;
  size_t row;

  inline void setRow(size_t r) {
    row = r;
  }

  radix_hash_value(const hyrise::storage::c_atable_ptr_t t, const size_t field) : dict(t->dictionaryAt(field)) {
    auto pair = getDataVector(t, field);
    vector = pair.first;
    f = pair.second;    
  }

  template<typename R>
  T operator()() {
    return std::hash<R>()(std::dynamic_pointer_cast<OrderPreservingDictionary<R>>(dict)->getValueForValueId(vector->get(f, row)));
  }
};




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
