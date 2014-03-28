// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueue.h"
#include "WSThreadLevelQueuesScheduler.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class WSThreadLevelQueuesScheduler;

template <class QUEUE>
class WSThreadLevelQueue;
typedef WSThreadLevelQueue<PriorityQueueType> WSThreadLevelPriorityQueue;
typedef WSThreadLevelQueue<BasicQueueType> WSThreadLevelBasicQueue;

/*
* A task queue with a number of threads executing tasks asynchronously;
* with work stealing
*/
template <class QUEUE>
class WSThreadLevelQueue : virtual public ThreadLevelQueue<QUEUE> {
  using ThreadLevelQueue<QUEUE>::_status;
  using ThreadLevelQueue<QUEUE>::_runQueue;
  using ThreadLevelQueue<QUEUE>::_threads;
  using ThreadLevelQueue<QUEUE>::_logger;

 protected:
  WSThreadLevelQueuesScheduler<QUEUE>* _scheduler;
  std::vector<std::shared_ptr<WSThreadLevelQueue> > _otherQueues;
  size_t _numberOfOtherQueues;

 public:
  WSThreadLevelQueue(WSThreadLevelQueuesScheduler<QUEUE>* scheduler, size_t threads)
      : ThreadLevelQueue<QUEUE>(threads), _scheduler(scheduler) {}
  ~WSThreadLevelQueue() {}

 public:
  /*
   * steal Task
   * */
  virtual std::shared_ptr<Task> stealTask() {
    std::shared_ptr<Task> task = nullptr;
    // dont steal tasks if thread is about to stop
    if (_status == ThreadLevelQueue<QUEUE>::RUN && _runQueue.unsafe_size() >= 1) {
      _runQueue.try_pop(task);
    }
    return task;
  }

  virtual void setOtherQueues() {
    auto all_queues = _scheduler->getTaskQueues();
    for (size_t i = 0; i < all_queues->size(); i++) {
      auto q = std::dynamic_pointer_cast<WSThreadLevelQueue>(all_queues->at(i));
      if (q != ThreadLevelQueue<QUEUE>::shared_from_this()) {
        _otherQueues.push_back(q);
      }
    }
    _numberOfOtherQueues = _otherQueues.size();
  }

 protected:
  virtual std::shared_ptr<Task> stealTasks() {
    std::shared_ptr<Task> task = nullptr;
    // quick check if scheduler is still running
    if (_numberOfOtherQueues > 0) {
      // we iterate over the queues provided by the scheduler
      // starting by 0 might create an imbalance - however, rand() is not thread-safe;
      // take WSCoreBoundNode, if that is an issue...
      int start = 0;
      for (size_t i = 0; i < _numberOfOtherQueues; i++) {
        task = _otherQueues.at((start + i) % _numberOfOtherQueues)->stealTask();
        if (task != nullptr) {
          LOG4CXX_DEBUG(_logger, "Task " << task->vname() << " was stolen from other queue");
          break;
        }
      }
    }
    return task;
  }
  /*
 * Is executed by dedicated thread to work the queue
 */
  virtual void executeTasks() {
    size_t retries = 0;
    // infinite thread loop
    while (true) {
      if (_status == ThreadLevelQueue<QUEUE>::TO_STOP) {
        break;
      }  // break out when asked
      std::shared_ptr<Task> task = nullptr;
      if (!_runQueue.try_pop(task)) {
        // if not, try to steal task
        if (!(task = stealTasks())) {
          if (retries++ < 10000) {
            if (retries > 300)
              std::this_thread::yield();
          } else {
            std::unique_lock<AbstractTaskScheduler::lock_t> locker(ThreadLevelQueue<QUEUE>::_lockqueue);
            if (_status != AbstractTaskScheduler::RUN)
              break;
            ThreadLevelQueue<QUEUE>::_queuecheck.wait(locker);
            retries = 0;
          }
        }
      }
      if (task) {
        retries = 0;
        (*task)();
        LOG4CXX_DEBUG(_logger, "Executed task " << task->vname());
        task->notifyDoneObservers();
      }
    }
  }
};
}
}