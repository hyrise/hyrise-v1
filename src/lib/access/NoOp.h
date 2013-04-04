// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_NOOP_H_
#define SRC_LIB_ACCESS_NOOP_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

// used to test protected methods
class ParallelExecutionTest_data_distribution_Test;

class NoOp : public _PlanOperation {
  friend class ParallelExecutionTest_data_distribution_Test;

public:
  virtual ~NoOp();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
};

}
}

#endif  // SRC_LIB_ACCESS_NOOP_H_
