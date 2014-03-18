// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueuesScheduler.h"
#include "CoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class CoreBoundQueuesScheduler;
typedef CoreBoundQueuesScheduler<PriorityQueueType> CoreBoundPriorityQueuesScheduler;
typedef CoreBoundQueuesScheduler<BasicQueueType> CoreBoundBasicQueuesScheduler;

/*
* 2-Level-Scheduler: This scheduler dispatches tasks to queues,
* each running a thread on a dedicated core
*/
template <class QUEUE>
class CoreBoundQueuesScheduler : virtual public ThreadLevelQueuesScheduler<QUEUE> {
  using ThreadLevelQueuesScheduler<QUEUE>::_queues;
  using ThreadLevelQueuesScheduler<QUEUE>::getNextQueue;
  using ThreadLevelQueuesScheduler<QUEUE>::_queueCount;
  using ThreadLevelQueuesScheduler<QUEUE>::_logger;

 protected:
  virtual void pushToQueue(const std::shared_ptr<Task>& task) {

    // check if task should be scheduled on specific core
    int core = task->getPreferredCore();
    if (core >= 0 && core < static_cast<int>(_queueCount)) {
      // potentially assigns task to a queue blocked by long running task
      _queues[core]->schedule(task);
      LOG4CXX_DEBUG(_logger, "Task " << task->vname() << " pushed to queue " << core);
    } else if (core == Task::NO_PREFERRED_CORE || core >= static_cast<int>(_queueCount)) {
      if (core < Task::NO_PREFERRED_CORE || core >= static_cast<int>(ThreadLevelQueuesScheduler<QUEUE>::_queueCount))
        // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
        LOG4CXX_WARN(
            _logger,
            "Tried to assign task " << task->vname() << " to core " << core
                                    << " which is not assigned to scheduler; assigned it to next available core");
      // lock queuesMutex to sync pushing to queue and incrementing next queue
      size_t q = getNextQueue();
      // simple strategy to avoid blocking of queues; check if queue is blocked - try a couple of times, otherwise
      // schedule on next queue
      size_t retries = 0;
      while (_queues[q]->blocked() && retries < 100) {
        q = getNextQueue();
        ++retries;
      }
      _queues[q]->schedule(task);
    }
  }

  virtual std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int core) {
    return std::make_shared<CoreBoundQueue<QUEUE>>(core, 1);
  }

 public:
  CoreBoundQueuesScheduler(int queues) : ThreadLevelQueuesScheduler<QUEUE>(queues) {};
  ~CoreBoundQueuesScheduler() {};

  virtual void schedule(const std::shared_ptr<Task>& task, int core) {
    task->setPreferredCore(core);
    ThreadLevelQueuesScheduler<QUEUE>::schedule(task);
  }
};
}
}
