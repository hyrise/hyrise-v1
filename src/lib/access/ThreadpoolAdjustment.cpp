// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ThreadpoolAdjustment.h"

#include "helper/Settings.h"

#include "taskscheduler/AbstractTaskScheduler.h"
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<ThreadpoolAdjustment>("ThreadpoolAdjustment");
}

ThreadpoolAdjustment::ThreadpoolAdjustment() : _size(1) {
}

ThreadpoolAdjustment::~ThreadpoolAdjustment() {
}

void ThreadpoolAdjustment::executePlanOperation() {
  AbstractTaskScheduler *scheduler = SharedScheduler::getInstance().getScheduler();
  if (scheduler != NULL) {
    scheduler->resize(_size);
    Settings::getInstance()->setThreadpoolSize(_size);
  } else {
    setState(OpFail);
    setErrorMessage("TaskScheduler is not of Type AbstractQueueBasedTaskScheduler and cannot be resized");
  }
}

std::shared_ptr<_PlanOperation> ThreadpoolAdjustment::parse(Json::Value &data) {
  std::shared_ptr<ThreadpoolAdjustment> threadpoolAdjustmentOp = std::make_shared<ThreadpoolAdjustment>();
  threadpoolAdjustmentOp->_size = data["size"].asUInt();
  return threadpoolAdjustmentOp;
}

const std::string ThreadpoolAdjustment::vname() {
  return "ThreadpoolAdjustment";
}

void ThreadpoolAdjustment::setThreadpoolSize(const size_t newSize) {
  _size = newSize;
}

}
}
