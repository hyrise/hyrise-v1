// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * SimpleTaskScheduler.h
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_SIMPLETASKSCHEDULER_H_
#define SRC_LIB_TASKSCHEDULER_SIMPLETASKSCHEDULER_H_

#include "taskscheduler/AbstractTaskScheduler.h"
#include <unordered_set>
#include <vector>
#include <queue>
#include <mutex>
#include <log4cxx/logger.h>
#include "taskscheduler/Task.h"
#include "taskscheduler/CoreBoundTaskQueue.h"
#include "helper/HwlocHelper.h"

/*
 * a simple task scheduler using threadspecific run queues;
 * each thread is pinned to a dedicated core;
 * tasks not ready for execution are placed in a central wait set
 */

template <class TaskQueue>
class SimpleTaskScheduler : public AbstractQueueBasedTaskScheduler<TaskQueue> {
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::task_queue_t task_queue_t;
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::task_queues_t task_queues_t;

public:
  SimpleTaskScheduler(const int queues = getNumberOfCoresOnSystem()): AbstractQueueBasedTaskScheduler<TaskQueue>() {
    // call resizeQueues here and not in AbstractQueueBasedTaskScheduler, as resizeQueue calls virtual createTaskQueue
    this->resize(queues);
  }

  virtual ~SimpleTaskScheduler() {
    std::lock_guard<std::mutex> lk2(this->_queuesMutex);
    task_queue_t *queue;
    for (unsigned i = 0; i < this->_taskQueues.size(); ++i) {
      queue =   this->_taskQueues[i];
      queue->stopQueue();
      delete queue;
    }
  }

protected:
  virtual void pushToQueue(std::shared_ptr<Task> task)
  {
    // check if task should be scheduled on specific core
    int core = task->getPreferredCore();

    // lock queuesMutex to manipulate queues
    std::lock_guard<std::mutex> lk2(this->_queuesMutex);
    if (core >= 0 && core < static_cast<int>(this->_queues)) {
      //potentially assigns task to a queue blocked by long running task
      this->_taskQueues[core]->push(task);
      LOG4CXX_DEBUG(this->_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " pushed to queue " << core);
    } else if (core == NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues)) {

      if (core < NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues))
        // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
        LOG4CXX_WARN(this->_logger, "Tried to assign task " << std::hex << (void *)task.get() << std::dec << " to core " << std::to_string(core) << " which is not assigned to scheduler; assigned it to next available core");

      // simple strategy to avoid blocking of queues; check if queue is blocked - try a couple of times, otherwise schedule on next queue
      size_t retries = 0;
      while (this->_taskQueues[this->_nextQueue]->blocked() && retries < 100) {
        this->_nextQueue = (this->_nextQueue + 1) % this->_queues;
        ++retries;
      }

      this->_taskQueues[this->_nextQueue]->push(task);
      //std::cout << "Task " << std::hex << (void * )task.get() << std::dec << " pushed to queue " << _nextQueue << std::endl;

      //round robin on cores
      this->_nextQueue = (this->_nextQueue + 1) % this->_queues;
    }
  }

  virtual void stopQueueAndRedistributeTasks(task_queue_t *queue, int queues) {
    std::queue<std::shared_ptr<Task> > tmp = queue->stopQueue();
    //redistribute tasks to other queues
    if (tmp.size() > 0) {
      int tmp_size = tmp.size();
      for (int i = 0; i < tmp_size; ++i) {
        // set preferred core to "NO_PREFERRED_CORE" (as queue with preferred core does not exist anymore / is used for other class of tasks)
        tmp.front()->setPreferredCore(NO_PREFERRED_CORE);
        pushToQueue(tmp.front());
        tmp.pop();
      }
    }
  }

  virtual task_queue_t *createTaskQueue(int core) {
    return new CoreBoundTaskQueue(core);
  }
};

#endif  // SRC_LIB_TASKSCHEDULER_SIMPLETASKSCHEDULER_H_
