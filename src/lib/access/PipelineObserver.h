// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PIPELINEOBSERVER_H_
#define SRC_LIB_ACCESS_PIPELINEOBSERVER_H_

#include "helper/types.h"
#include "helper/pipelining.h"

#include <log4cxx/logger.h>

#include "taskscheduler/SharedScheduler.h"
#include "access/system/ResponseTask.h"
#include "access/system/PlanOperation.h"

#include <algorithm>

namespace hyrise {
namespace access {

namespace {
log4cxx::LoggerPtr _observerLogger(log4cxx::Logger::getLogger("pipelining.PipelineObserver"));
}

/*
 * We use the Curiously Recurring Template Pattern below.
 * In order to still be able to dynamic_cast to ALL PipelineObservers,
 * regardless of their template parameter, we had to introduce this abstract class.
 */
class AbstractPipelineObserver {
 public:
  virtual ~AbstractPipelineObserver();
  virtual void notifyNewChunk(storage::c_aresource_ptr_t chunk) = 0;
};

/*
 * Derive from this class to be able to receive (hash) chunks.
 * Fully-pipelining ops have to derive from PipelineObserver and PipelineEmitter.
 * We use CRTP since we have to mixin functionality to some PlanOperations and
 * still be able to use the PlanOperation interface. We did not want to introduce
 * diamond-shaped inheritance. Therefore, we went with CRTP.
 * See docs in "Implementation Details" > "Pipelining"
 */
template <class T>
class PipelineObserver : public AbstractPipelineObserver {
 public:
  virtual void notifyNewChunk(storage::c_aresource_ptr_t chunk) {
    auto opName = static_cast<T*>(this)->planOperationName();
    auto opId = static_cast<T*>(this)->getOperatorId();

    LOG4CXX_DEBUG(_observerLogger, opId << ": notifyNewChunk");

    auto clone = std::static_pointer_cast<T>(this->clone());
    clone->setPlanOperationName(opName + "_chunk");
    clone->setOperatorId(getChunkIdentifier(opId));

    // input for this new instance is chunk
    clone->addInput(chunk);

    // set dependencies
    // a) this new chunk task is dependency to all successors.
    auto successors = static_cast<T*>(this)->template getAllSuccessorsOf<PlanOperation>();
    std::for_each(successors.begin(), successors.end(), [&clone](std::shared_ptr<PlanOperation>& obs) {
      obs->addDependency(clone);
    });

    // b) allow to add additional dependencies, e.g. hashtable for probes
    addCustomDependencies(clone);

    // schedule task
    auto scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();
    if (const auto& responseTask = static_cast<T*>(this)->getResponseTask()) {
      responseTask->registerPlanOperation(clone);
    }
    scheduler->schedule(clone);
  }

 protected:
  /*
   * The default implementation sets the newly created task
   * as a dependency of all successors.
   * If you need to implement custom, additional dependencies,
   * do it here.
   */
  virtual void addCustomDependencies(taskscheduler::task_ptr_t newChunkTask) {}
  virtual std::shared_ptr<AbstractPipelineObserver> clone() = 0;
};
}
}

#endif  // SRC_LIB_ACCESS_PIPELINEOBSERVER_H_
