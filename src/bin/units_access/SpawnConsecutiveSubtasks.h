// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SPAWNCONSECUTIVESUBTASKS_H
#define SRC_LIB_ACCESS_SPAWNCONSECUTIVESUBTASKS_H

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements a testing operation that spawns a number of subtasks executed consecutively.
/// The successortask will be executed after the last subtask has finished
class SpawnConsecutiveSubtasks : public _PlanOperation {
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

#endif // SRC_LIB_ACCESS_SPAWNCONSECUTIVESUBTASKS_H
