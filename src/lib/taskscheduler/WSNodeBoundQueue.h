// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "WSThreadLevelQueue.h"
#include "NodeBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class WSNodeBoundQueue;
typedef WSNodeBoundQueue<PriorityQueueType> WSNodeBoundPriorityQueue;
typedef WSNodeBoundQueue<BasicQueueType> WSNodeBoundBasicQueue;

/*
* A Queue that asynchronously executes the tasks by threads
* that are bound to a given node. If empty the queue tries to
* steal tasks from other known nodes
*/
template <class QUEUE>
class WSNodeBoundQueue : virtual public WSThreadLevelQueue<QUEUE>, virtual public NodeBoundQueue<QUEUE> {

  using NodeBoundQueue<QUEUE>::_node;
  using WSThreadLevelQueue<QUEUE>::_otherQueues;
  using WSThreadLevelQueue<QUEUE>::_numberOfOtherQueues;

 protected:
 public:
  WSNodeBoundQueue(WSThreadLevelQueuesScheduler<QUEUE>* scheduler, size_t core, size_t threads)
      : ThreadLevelQueue<QUEUE>(threads),
        WSThreadLevelQueue<QUEUE>(scheduler, threads),
        NodeBoundQueue<QUEUE>(core, threads) {}
  ~WSNodeBoundQueue() {}

  std::shared_ptr<Task> stealTasks() {
    // WSThreadLevelQueuesScheduler::scheduler_status_t status = 0;
    // LOG4CXX_DEBUG(_logger, "Try to steal tasks");
    std::shared_ptr<Task> task = nullptr;
    // quick check if scheduler is still running
    if (_numberOfOtherQueues > 0) {
      // we iterate over the queues provided by the scheduler with a random start position to distribute queues.
      int start = _node;
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