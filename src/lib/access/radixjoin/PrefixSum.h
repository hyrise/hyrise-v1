// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PREFIXSUM_H_
#define SRC_LIB_ACCESS_PREFIXSUM_H_

#include "access/system/ParallelizablePlanOperation.h"

#include "storage/FixedLengthVector.h"

namespace hyrise {
namespace access {

class PrefixSum : public ParallelizablePlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void splitInput();

private:
  typedef std::shared_ptr<storage::FixedLengthVector<storage::value_id_t>> vec_ref_t;
  storage::value_id_t sumForIndex(const size_t ivec_size,
                                  const std::vector<vec_ref_t> &ivecs,
                                  const size_t index) const;
  storage::value_id_t sumForIndexPrev(const size_t ivec_size,
                                      const std::vector<vec_ref_t>& ivecs,
                                      const size_t index) const;
};

class MergePrefixSum : public PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
};

}
}

#endif  // SRC_LIB_PREFIXSUM_H_
