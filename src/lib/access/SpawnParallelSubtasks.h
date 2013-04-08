// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SPAWNPARALLELSUBTASKS_H
#define SRC_LIB_ACCESS_SPAWNPARALLELSUBTASKS_H

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements the distinct operator for any kind of input table.
/// It has linear complexity since it scans the attribute and retrieves
/// all distinct valueIds and builds the result.
class SpawnParallelSubtasks : public _PlanOperation {
public:
  virtual ~SpawnParallelSubtasks();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
};

}
}

#endif // SRC_LIB_ACCESS_SPAWNPARALLELSUBTASKS_H
