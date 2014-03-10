// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "ThreadLevelQueuesScheduler.h"
#include "WSThreadLevelQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class WSThreadLevelQueuesScheduler;
typedef WSThreadLevelQueuesScheduler<PriorityQueueType> WSThreadLevelPriorityQueuesScheduler;
typedef WSThreadLevelQueuesScheduler<BasicQueueType> WSThreadLevelBasicQueuesScheduler;

/*
* 2-Level-Scheduler: This scheduler dispatches tasks to queues,
* each running a thread for task execution; with work stealing
*/
template <class QUEUE>
class WSThreadLevelQueuesScheduler : virtual public ThreadLevelQueuesScheduler<QUEUE> {
  using ThreadLevelQueuesScheduler<QUEUE>::_queues;
  using ThreadLevelQueuesScheduler<QUEUE>::getNextQueue;
  using ThreadLevelQueuesScheduler<QUEUE>::_queueCount;
  using ThreadLevelQueuesScheduler<QUEUE>::_status;

  virtual void pushToQueue(std::shared_ptr<Task> task) {
    size_t q = getNextQueue();
    _queues[q]->schedule(task);
  }

 public:
  WSThreadLevelQueuesScheduler(int queues) : ThreadLevelQueuesScheduler<QUEUE>(queues) {};
  ~WSThreadLevelQueuesScheduler() {};

  const typename ThreadLevelQueuesScheduler<QUEUE>::task_queues_t* getTaskQueues() const {
    // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return
    // nullptr
    // otherwise copy queues from _taskQueues and cast to WSThreadLevelQueues
    if (this->_status == AbstractTaskScheduler::RESIZING || this->_status == AbstractTaskScheduler::TO_STOP ||
        this->_status == AbstractTaskScheduler::STOPPED)
      return nullptr;
    // return const reference to task queues
    return &_queues;
  }


  void init() {
    for (size_t i = 0; i < _queueCount; i++) {
      _queues.push_back(createTaskQueue(i));
      _queues[i]->init();
    }
    for (unsigned i = 0; i < _queues.size(); ++i) {
      std::dynamic_pointer_cast<WSThreadLevelQueue<QUEUE>>(_queues[i])->setOtherQueues();
    }
    _status = AbstractTaskScheduler::RUN;
  }


  std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int core) {
    return std::make_shared<WSThreadLevelQueue<QUEUE>>(this, 1);
  }
};
}
}
