// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "WSThreadLevelQueuesScheduler.h"
#include "CoreBoundQueuesScheduler.h"
#include "WSCoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

template <class QUEUE>
class WSCoreBoundQueuesScheduler;
typedef WSCoreBoundQueuesScheduler<PriorityQueueType> WSCoreBoundPriorityQueuesScheduler;
typedef WSCoreBoundQueuesScheduler<BasicQueueType> WSCoreBoundBasicQueuesScheduler;

/*
* 2-Level-Scheduler: This scheduler dispatches tasks to queues,
* each running a thread on a dedicated core; with workstealing
*/
template <class QUEUE>
class WSCoreBoundQueuesScheduler : virtual public ThreadLevelQueuesScheduler<QUEUE>,
                                   virtual public WSThreadLevelQueuesScheduler<QUEUE>,
                                   virtual public CoreBoundQueuesScheduler<QUEUE> {

  using ThreadLevelQueuesScheduler<QUEUE>::_queues;
  using ThreadLevelQueuesScheduler<QUEUE>::getNextQueue;
  using ThreadLevelQueuesScheduler<QUEUE>::_queueCount;

 public:
  WSCoreBoundQueuesScheduler(int queues)
      : ThreadLevelQueuesScheduler<QUEUE>(queues),
        WSThreadLevelQueuesScheduler<QUEUE>(queues),
        CoreBoundQueuesScheduler<QUEUE>(queues) {};
  ~WSCoreBoundQueuesScheduler() {};

  std::shared_ptr<typename ThreadLevelQueuesScheduler<QUEUE>::task_queue_t> createTaskQueue(int core) {
    return std::make_shared<WSCoreBoundQueue<QUEUE>>(this, core, 1);
  }

 protected:
  virtual void pushToQueue(std::shared_ptr<Task> task) {
    int core = task->getPreferredCore();
    if (core >= 0 && core < static_cast<int>(_queueCount)) {
      // push task to queue that runs on given core
      _queues[core]->schedule(task);
      // LOG4CXX_DEBUG(this->_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " pushed to queue " <<
      // core);
    } else if (core == Task::NO_PREFERRED_CORE || core >= static_cast<int>(_queueCount)) {
      // if (core < Task::NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues))
      // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
      // LOG4CXX_WARN(this->_logger, "Tried to assign task " << std::hex << (void *)task.get() << std::dec << " to core
      // " << std::to_string(core) << " which is not assigned to scheduler; assigned it to next available core");
      // push task to next queue
      // std::cout << "Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " pushed to queue " <<
      // this->_nextQueue << std::endl;
      // round robin on cores
      size_t q = getNextQueue();
      _queues[q]->schedule(task);
    }
  }
};
}
}
