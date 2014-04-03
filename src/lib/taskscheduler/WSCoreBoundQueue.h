// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "WSThreadLevelQueue.h"
#include "CoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class WSCoreBoundQueue;
typedef WSCoreBoundQueue<PriorityQueueType> WSCoreBoundPriorityQueue;
typedef WSCoreBoundQueue<BasicQueueType> WSCoreBoundBasicQueue;

/*
* A Queue that asynchronously executes the tasks by threads
* that are bound to a given core. If empty the queue tries to
* steal tasks from other known nodes
*/
template <class QUEUE>
class WSCoreBoundQueue : virtual public WSThreadLevelQueue<QUEUE>, virtual public CoreBoundQueue<QUEUE> {

  using CoreBoundQueue<QUEUE>::_core;
  using WSThreadLevelQueue<QUEUE>::_otherQueues;
  using WSThreadLevelQueue<QUEUE>::_numberOfOtherQueues;

 protected:
 public:
  WSCoreBoundQueue(WSThreadLevelQueuesScheduler<QUEUE>* scheduler, size_t core, size_t threads)
      : ThreadLevelQueue<QUEUE>(threads),
        WSThreadLevelQueue<QUEUE>(scheduler, threads),
        CoreBoundQueue<QUEUE>(core, threads) {}
  ~WSCoreBoundQueue() {}

  std::shared_ptr<Task> stealTasks() {
    // WSThreadLevelQueuesScheduler::scheduler_status_t status = 0;
    // LOG4CXX_DEBUG(_logger, "Try to steal tasks");
    std::shared_ptr<Task> task = nullptr;
    // quick check if scheduler is still running
    if (_numberOfOtherQueues > 0) {
      // we iterate over the queues provided by the scheduler with a random start position to distribute queues.
      int start = _core;
      for (size_t i = 0; i < _numberOfOtherQueues; i++) {
        // std::cout << "Workerhread tries to steal task from queue " << i<< "  scheduler status " << status <<
        // std::endl;
        task = _otherQueues.at((start + i) % _numberOfOtherQueues)->stealTask();
        if (task != nullptr) {
          // std::cout << "WorkerThreadLevelQueue stole task" << std::endl;
          // LOG4CXX_DEBUG(_logger, "WorkerThreadLevelQueue stole task");
          break;
        }
      }
    }
    return task;
  }
};
}
}
