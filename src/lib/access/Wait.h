// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_WAIT_H_
#define SRC_LIB_ACCESS_WAIT_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class Wait : public PlanOperation {
 public:
  Wait(std::chrono::milliseconds wait);
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
 private:
  std::chrono::milliseconds _wait;
};

}}

#endif  // SRC_LIB_ACCESS_NOOP_H_
