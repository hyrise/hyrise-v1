// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TASKSCHEDULERADJUSTMENT_H_
#define SRC_LIB_ACCESS_TASKSCHEDULERADJUSTMENT_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// This operation offers interfaces to adjust the configuration of the
/// task scheduler. Use this as long as there is no dedicated scheduling unit.
class TaskSchedulerAdjustment : public _PlanOperation {
public:
  TaskSchedulerAdjustment();
  virtual ~TaskSchedulerAdjustment();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
  void setThreadpoolSize(const size_t newSize);

private:
  size_t _size;
};

}
}

#endif  // SRC_LIB_ACCESS_TASKSCHEDULERADJUSTMENT_H_
