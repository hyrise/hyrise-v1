// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_ABSTRACTPLANOPTRANSFORMATION_H
#define SRC_LIB_ACCES_ABSTRACTPLANOPTRANSFORMATION_H

#include <json.h>
#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class AbstractPlanOpTransformation {
 public:
  AbstractPlanOpTransformation() {};
  virtual ~AbstractPlanOpTransformation() {};

  virtual void transform(Json::Value& op, const std::string& operatorId, Json::Value& query) = 0;

  static std::string name() { return "RadixJoin"; }
};
}
}

#endif  // SRC_LIB_ACCES_ABSTRACTPLANOPTRANSFORMATION_H
