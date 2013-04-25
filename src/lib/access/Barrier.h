// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_BARRIER_H_
#define SRC_LIB_ACCESS_BARRIER_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class Barrier : public _PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
};

}
}

#endif // SRC_LIB_ACCESS_BARRIER_H_
