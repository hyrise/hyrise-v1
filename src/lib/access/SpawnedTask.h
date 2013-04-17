// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SPAWNEDTASK_H
#define SRC_LIB_ACCESS_SPAWNEDTASK_H

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements an operation doing nothing but sleeping for a short random time
/// It's designed for testing the task spawning
class SpawnedTask : public _PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
};

}
}

#endif // SRC_LIB_ACCESS_SPAWNEDTASK_H
