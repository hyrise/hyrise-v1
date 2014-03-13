// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueuesScheduler.h"
#include "NodeBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class NodeBoundQueuesScheduler;
typedef NodeBoundQueuesScheduler<PriorityQueueType> NodeBoundPriorityQueuesScheduler;
typedef NodeBoundQueuesScheduler<BasicQueueType> NodeBoundBasicQueuesScheduler;

/*
* 2-Level-Scheduler: This scheduler dispatches tasks to queues,
* each running a provided number of threads on a dedicated node.
*
* Note that the "ownership" of queries is transferred, e.g., a
* dispatched RequestParseTask will submit generated tasks to the node
* level scheduler
*/
template <class QUEUE>
class NodeBoundQueuesScheduler : virtual public ThreadLevelQueuesScheduler<QUEUE> {
  using ThreadLevelQueuesScheduler<QUEUE>::_queues;
  using ThreadLevelQueuesScheduler<QUEUE>::getNextQueue;
  using ThreadLevelQueuesScheduler<QUEUE>::_queueCount;
  using ThreadLevelQueuesScheduler<QUEUE>::_status;

 protected:
  int _threadsPerNode;
  int _totalThreads;

  virtual std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int node) {
    return std::make_shared<NodeBoundQueue<QUEUE>>(node, 1);
  }

 public:
  NodeBoundQueuesScheduler(int queues) : ThreadLevelQueuesScheduler<QUEUE>(queues), _totalThreads(queues) {};
  ~NodeBoundQueuesScheduler() {};

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
    _status = AbstractTaskScheduler::RUN;
  }

  /*
   * schedule a task for execution
   */
  void schedule(const std::shared_ptr<Task>& task) {
    int nextQueue = getNextQueue();
    // Set scheduler for RequestParseTasks, so that new tasks arrive directly at the node level scheduler
    if (task->vname() == "RequestParseTask") {
      task->setScheduler(_queues[nextQueue]);
    }

    // push task to scheduler (round robin)
    _queues[nextQueue]->schedule(task);
  }

  virtual std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int node,
                                                                                                    int threads) {
    return std::make_shared<NodeBoundQueue<QUEUE>>(node, threads);
  }

  /**
 * get number of worker
 */
  virtual size_t getNumberOfWorker() const { return _totalThreads; }
};
}
}
