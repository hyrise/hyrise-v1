// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_NOOP_H_
#define SRC_LIB_ACCESS_NOOP_H_

#include "PlanOperation.h"

class NoOp : public _PlanOperation {
 public:

  NoOp() {
    
  }

  virtual ~NoOp() {
  }

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "NoOp";
  }
  const std::string vname() {
    return "NoOp";
  }
};

#endif  // SRC_LIB_ACCESS_NOOP_H_

