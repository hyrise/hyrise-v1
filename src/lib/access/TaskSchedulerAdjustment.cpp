// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/TaskSchedulerAdjustment.h"

#include "helper/Settings.h"

#include "taskscheduler/AbstractTaskScheduler.h"
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<TaskSchedulerAdjustment>("TaskSchedulerAdjustment");
}

TaskSchedulerAdjustment::TaskSchedulerAdjustment() : _size(1) {
}

TaskSchedulerAdjustment::~TaskSchedulerAdjustment() {
}

void TaskSchedulerAdjustment::executePlanOperation() {
  AbstractTaskScheduler *scheduler = SharedScheduler::getInstance().getScheduler();
  if (scheduler != NULL) {
    scheduler->resize(_size);
    Settings::getInstance()->setThreadpoolSize(_size);
  } else {
    setState(OpFail);
    setErrorMessage("TaskScheduler is not of Type AbstractQueueBasedTaskScheduler and cannot be resized");
  }
}

std::shared_ptr<_PlanOperation> TaskSchedulerAdjustment::parse(Json::Value &data) {
  std::shared_ptr<TaskSchedulerAdjustment> taskSchedulerAdjustmentOp = std::make_shared<TaskSchedulerAdjustment>();
  taskSchedulerAdjustmentOp->_size = data["size"].asUInt();
  return taskSchedulerAdjustmentOp;
}

const std::string TaskSchedulerAdjustment::vname() {
  return "TaskSchedulerAdjustment";
}

void TaskSchedulerAdjustment::setThreadpoolSize(const size_t newSize) {
  _size = newSize;
}

}
}
