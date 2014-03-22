// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_RADIXCLUSTER_H_
#define SRC_LIB_ACCESS_RADIXCLUSTER_H_

#include "access/system/ParallelizablePlanOperation.h"

#include "Histogram.h"

namespace hyrise {
namespace access {

class CreateRadixTable : public PlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
};

/// This class provides a straight-forward implementation of a radix-clustering
/// algorithm. The input to this operation are the required number of bits for
/// the clustering, the prefix sum table from the histogram, the cluster field
/// and the input table to cluster
class RadixCluster : public ParallelizablePlanOperation {
 public:
  RadixCluster();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  template <typename T>
  void executeClustering();
  void setPart(const size_t p);
  void setCount(const size_t c);
  void setPartInfo(const int32_t p, const int32_t n);
  void setBits(const uint32_t b, const uint32_t sig = 0);
  uint32_t bits() const;
  uint32_t significantOffset() const;

 private:
  uint32_t _bits;
  uint32_t _significantOffset;
  size_t _start;
  size_t _stop;
  size_t _part;
  size_t _count;
};

template <typename T>
void RadixCluster::executeClustering() {
  const auto& tab = getInputTable();
  auto tableSize = tab->size();
  auto field = _field_definition[0];

  // Result Vector
  const auto& result = getInputTable(1);

  // Get the prefix sum from the input
  const auto& prefix_sum = getInputTable(2);
  const auto& data_prefix_sum = std::dynamic_pointer_cast<storage::AbstractFixedLengthVector<value_id_t>>(
      getFixedDataVector(prefix_sum).first->copy());

  const auto& data_hash = getFixedDataVector(result).first;
  const auto& data_pos = getFixedDataVector(result, 1).first;

  // Calculate start stop
  _start = 0;
  _stop = tableSize;
  if (_count > 0) {
    _start = (tableSize / _count) * _part;
    _stop = (_count - 1) == _part ? tableSize : (tableSize / _count) * (_part + 1);
  }

  _executeRadixHashing<T>(tab, field, _start, _stop, bits(), significantOffset(), data_prefix_sum, data_hash, data_pos);

  addResult(result);
}

/// Second pass on the previously radix clustered table to create the final output
/// table. Input to this operation is the result of the first pass and the second
/// pass of the prefix sum calculation
class RadixCluster2ndPass : public ParallelizablePlanOperation {
 public:
  RadixCluster2ndPass();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  void setBits1(const uint32_t b, const uint32_t sig = 0);
  void setBits2(const uint32_t b, const uint32_t sig = 0);
  void setPart(const size_t p);
  void setCount(const size_t c);

 private:
  uint32_t _bits1;
  uint32_t _significantOffset1;
  uint32_t _bits2;
  uint32_t _significantOffset2;
  size_t _part;
  size_t _count;
};
}
}

#endif  // SRC_LIB_ACCESS_RADIXCLUSTER_H_
