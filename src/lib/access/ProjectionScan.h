// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PROJECTIONSCAN_H_
#define SRC_LIB_ACCESS_PROJECTIONSCAN_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class ProjectionScan : public PlanOperation {
public:
  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
};

}
}

#endif  // SRC_LIB_ACCESS_PROJECTIONSCAN_H_
