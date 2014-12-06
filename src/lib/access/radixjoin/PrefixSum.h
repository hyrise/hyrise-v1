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
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  void splitInput();

 private:
  // use finalized type
  typedef storage::FixedLengthVector<storage::value_id_t> vec_t;
  typedef std::shared_ptr<vec_t> vec_ref_t;
};

class MergePrefixSum : public PlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
};
}
}

#endif  // SRC_LIB_PREFIXSUM_H_
