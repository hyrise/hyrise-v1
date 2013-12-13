/*
 * CoreBoundPriorityQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "taskscheduler/Task.h"
#include "taskscheduler/CoreBoundPriorityQueue.h"
#include "taskscheduler/AbstractCoreBoundQueuesScheduler.h"
#include <memory>
#include <unordered_set>
#include <log4cxx/logger.h>

namespace hyrise {
namespace taskscheduler {

/**
 * a task scheduler with thread specific queues
 */
class CoreBoundPriorityQueuesScheduler : public AbstractCoreBoundQueuesScheduler {
  int count = 0; 
   /**
    * push ready task to the next queue
    */
   virtual void pushToQueue(std::shared_ptr<Task> task);

   /*
    * create a new task queue
    */
   virtual task_queue_t *createTaskQueue(int core);
   /*
    * set next queue to push task to
    */
    virtual size_t getNextQueue();

public:
  CoreBoundPriorityQueuesScheduler(int queues = getNumberOfCoresOnSystem());
  virtual ~CoreBoundPriorityQueuesScheduler();

};

} } // namespace hyrise::taskscheduler

