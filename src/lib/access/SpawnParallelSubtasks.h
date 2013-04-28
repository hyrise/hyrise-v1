// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SPAWNPARALLELSUBTASKS_H
#define SRC_LIB_ACCESS_SPAWNPARALLELSUBTASKS_H

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

///This class implements an operation that spawns a number of subtasks for testing purposes.
///The successor tasks will be executed after all spawned tasks are done
class SpawnParallelSubtasks : public _PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();

  void setNumberOfSpawns(size_t number);

private:
  size_t m_numberOfSpawns;
};

}
}

#endif // SRC_LIB_ACCESS_SPAWNPARALLELSUBTASKS_H
