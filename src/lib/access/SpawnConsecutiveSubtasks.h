// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SPAWNCONSECUTIVESUBTASKS_H
#define SRC_LIB_ACCESS_SPAWNCONSECUTIVESUBTASKS_H

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements the distinct operator for any kind of input table.
/// It has linear complexity since it scans the attribute and retrieves
/// all distinct valueIds and builds the result.
class SpawnConsecutiveSubtasks : public _PlanOperation {
public:
  virtual ~SpawnConsecutiveSubtasks();

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
