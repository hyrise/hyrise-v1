// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_NESTEDLOOPEQUIJOIN_H_
#define SRC_LIB_ACCESS_NESTEDLOOPEQUIJOIN_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// This is a Join Plan Operation based on the Nestedloop algorithm.
/// The following input tables are expected:
/// input table 0: left table
/// input table 1: left hashtable (hashvalue, pos)
/// input table 2: left prefixsum table for whole table
/// input table 3: right table
/// input table 4: right hashtable (hashvalue, pos)
/// input table 5: right prefixsum table for whole table
///
/// The following attributes need to be provided:
/// list of partitions to work on
/// number of right-most bits for the first hash
/// number of next right-most bits for the second hash
///
/// This Operation is used for the Radix Join
/// implementation
class NestedLoopEquiJoin : public PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setBits1(const uint32_t b);
  void setBits2(const uint32_t b);
  uint32_t bits1() const;
  uint32_t bits2() const;
  void addPartition(const int p);

private:
  uint32_t _bits1;
  uint32_t _bits2;
  std::vector<int> _partitions;
};

}
}

#endif  // SRC_LIB_NESTEDLOOPEQUIJOIN_H_
