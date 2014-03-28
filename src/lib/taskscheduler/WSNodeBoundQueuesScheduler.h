// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "WSThreadLevelQueuesScheduler.h"
#include "NodeBoundQueuesScheduler.h"
#include "WSNodeBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class WSNodeBoundQueuesScheduler;
typedef WSNodeBoundQueuesScheduler<PriorityQueueType> WSNodeBoundPriorityQueuesScheduler;
typedef WSNodeBoundQueuesScheduler<BasicQueueType> WSNodeBoundBasicQueuesScheduler;
/*
* 2-Level-Scheduler: This scheduler dispatches tasks to queues,
* each running threads on a dedicated node; with workstealing
*/
template <class QUEUE>
class WSNodeBoundQueuesScheduler : virtual public ThreadLevelQueuesScheduler<QUEUE>,
                                   virtual public WSThreadLevelQueuesScheduler<QUEUE>,
                                   virtual public NodeBoundQueuesScheduler<QUEUE> {

  using ThreadLevelQueuesScheduler<QUEUE>::_queues;
  using ThreadLevelQueuesScheduler<QUEUE>::_status;
  using ThreadLevelQueuesScheduler<QUEUE>::getNextQueue;
  using ThreadLevelQueuesScheduler<QUEUE>::_queueCount;
  using NodeBoundQueuesScheduler<QUEUE>::_threadsPerNode;
  using NodeBoundQueuesScheduler<QUEUE>::_totalThreads;

 public:
  WSNodeBoundQueuesScheduler(int queues)
      : ThreadLevelQueuesScheduler<QUEUE>(queues),
        WSThreadLevelQueuesScheduler<QUEUE>(queues),
        NodeBoundQueuesScheduler<QUEUE>(queues) {};
  ~WSNodeBoundQueuesScheduler() {};

  virtual void init() {
    // get Number of Nodes
    _queueCount = getNumberOfNodesOnSystem();
    _threadsPerNode = _totalThreads / _queueCount;

    // create Processor Level Schedulers
    for (size_t i = 0; i < _queueCount; i++) {
      auto q = createTaskQueue(i, _threadsPerNode);
      q->init();
      _queues.push_back(q);
    }
    for (unsigned i = 0; i < _queues.size(); ++i) {
      std::dynamic_pointer_cast<WSThreadLevelQueue<QUEUE>>(_queues[i])->setOtherQueues();
    }
    _status = AbstractTaskScheduler::RUN;
  }

  virtual std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int core) {
    return std::make_shared<WSNodeBoundQueue<QUEUE>>(this, core, 1);
  }

  virtual std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int core,
                                                                                                    int threads) {
    return std::make_shared<WSNodeBoundQueue<QUEUE>>(this, core, threads);
  }
};
}
}
