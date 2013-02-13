// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PREFIXSUM_H_
#define SRC_LIB_ACCESS_PREFIXSUM_H_

#include "PlanOperation.h"

namespace hyrise {
namespace access {

class PrefixSum : public _PlanOperation {

  typedef std::shared_ptr<FixedLengthVector<value_id_t>> vec_ref_t;

  value_id_t sumForIndex(const size_t ivec_size, std::vector<vec_ref_t>& ivecs, size_t index);
  value_id_t sumForIndexPrev(const size_t ivec_size, std::vector<vec_ref_t>& ivecs, size_t index);

    

public:

  PrefixSum() {
    
  }

  virtual ~PrefixSum() {
  }

  void executePlanOperation();

  void splitInput(){};

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;

  const std::string vname() {
    return "PrefixSum";
  }

};

class MergePrefixSum : public _PlanOperation {

public:

  virtual ~MergePrefixSum() {}

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data) {
    return std::make_shared<MergePrefixSum>();
  }
  static bool is_registered;

  const std::string vname() {
    return "MergePrefixSum";
  }

};


}}
#endif  // SRC_LIB_PREFIXSUM_H_
