// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueue.h"

namespace hyrise {
namespace taskscheduler {

template<class Queue> class DynamicTaskQueue;
typedef DynamicTaskQueue<PriorityQueueType> DynamicTaskPriorityQueue;
typedef DynamicTaskQueue<BasicQueueType> DynamicTaskBasicQueue;

/*
* A scheduler that supports the feature of dynamic parallelism
* once a task is ready to run, the applyDynamicParallelization method is 
* called 
*/
template<class QUEUE>
class DynamicTaskQueue : public ThreadLevelQueue<QUEUE> {
  using ThreadLevelQueue<QUEUE>::_runQueue;

protected:
  size_t _maxTaskSize;
public:
  DynamicTaskQueue(size_t threads): ThreadLevelQueue<QUEUE>(threads), _maxTaskSize(0) {}
  ~DynamicTaskQueue(){}

  virtual void notifyReady(std::shared_ptr<Task> task){
    if (task->isDynamic()) {
      auto dynamicCount = task->determineDynamicCount(_maxTaskSize);
      auto tasks = task->applyDynamicParallelization(dynamicCount);
      for (const auto& i : tasks) {
        if (i->isReady()) {
          _runQueue.push(i);
        } else {   
          i->addReadyObserver(ThreadLevelQueue<QUEUE>::shared_from_this());
        }
      }
    } else { // task is not dynamic
      _runQueue.push(task);
    }
  }    

  virtual void schedule(std::shared_ptr<Task> task){
    if (task->isDynamic() && task->isReady()) {
      uint dynamicCount = task->determineDynamicCount(_maxTaskSize);
      auto tasks = task->applyDynamicParallelization(dynamicCount);
      for (const auto& i : tasks) {
        ThreadLevelQueue<QUEUE>::schedule(i);
      }
    } else {
      ThreadLevelQueue<QUEUE>::schedule(task);
    }
  }

  void setMaxTaskSize(size_t maxTaskSize) {
    _maxTaskSize = maxTaskSize;
  }
};

}}