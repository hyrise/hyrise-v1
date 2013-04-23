// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PROJECTIONSCAN_H_
#define SRC_LIB_ACCESS_PROJECTIONSCAN_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class ProjectionScan : public _PlanOperation {
public:
  ProjectionScan();
  explicit ProjectionScan(storage::field_list_t *fields);
  virtual ~ProjectionScan();

  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
};

}
}

#endif  // SRC_LIB_ACCESS_PROJECTIONSCAN_H_
