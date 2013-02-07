#include "ThreadpoolAdjustment.h"
#include <helper/Settings.h>
#include "taskscheduler/SimpleTaskScheduler.h"

bool ThreadpoolAdjustment::is_registered = QueryParser::registerPlanOperation<ThreadpoolAdjustment>();

std::shared_ptr<_PlanOperation> ThreadpoolAdjustment::parse(Json::Value &data) {
  std::shared_ptr<ThreadpoolAdjustment> threadpoolAdjustmentOp = std::make_shared<ThreadpoolAdjustment>();
  threadpoolAdjustmentOp->size = data["size"].asUInt();
  return threadpoolAdjustmentOp;
}

void ThreadpoolAdjustment::executePlanOperation() {

  AbstractTaskScheduler *scheduler = SharedScheduler::getInstance().getScheduler();
  if (scheduler != NULL) {
    scheduler->resize(size);
    Settings::getInstance()->setThreadpoolSize(size);
  } else {
    setState(OpFail);
    setErrorMessage("TaskScheduler is not of Type AbstractQueueBasedTaskScheduler and cannot be resized");
  }
}

void ThreadpoolAdjustment::setThreadpoolSize(const size_t newSize) {
  this->size = newSize;
}

