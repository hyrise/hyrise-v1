// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TASKSCHEDULERADJUSTMENT_H_
#define SRC_LIB_ACCESS_TASKSCHEDULERADJUSTMENT_H_

#include <access/PlanOperation.h>
#include <taskscheduler.h>

/*  This operation offers interfaces to adjust the configuration of the
    task scheduler. Use this as long as there is no dedicated scheduling unit.  */
class TaskSchedulerAdjustment : public _PlanOperation {
  size_t _size;
 public:
  static bool is_registered;

  TaskSchedulerAdjustment() :
      _size(1) {}

  virtual ~TaskSchedulerAdjustment() {
  }

  static std::string name() {
    return "TaskSchedulerAdjustment";
  }
  const std::string vname() {
    return "TaskSchedulerAdjustment";
  }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  void executePlanOperation();

  //  Set the maximum number of parallel executable operation tasks.
  void setThreadpoolSize(const size_t newSize);
};

#endif  // SRC_LIB_ACCESS_TASKSCHEDULERADJUSTMENT_H_

