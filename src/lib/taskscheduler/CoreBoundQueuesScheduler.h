/*
 * CoreBoundQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "taskscheduler/Task.h"
#include "taskscheduler/CoreBoundQueue.h"
#include "taskscheduler/AbstractCoreBoundQueuesScheduler.h"
#include <memory>
#include <unordered_set>
#include <log4cxx/logger.h>

namespace hyrise {
namespace taskscheduler {

/**
 * a task scheduler with thread specific queues
 */
class CoreBoundQueuesScheduler : public AbstractCoreBoundQueuesScheduler {

   /**
    * push ready task to the next queue
    */
   virtual void pushToQueue(std::shared_ptr<Task> task);

   /*
    * create a new task queue
    */
   virtual task_queue_t *createTaskQueue(int core);

public:
  CoreBoundQueuesScheduler(int queues = getNumberOfCoresOnSystem());
  virtual ~CoreBoundQueuesScheduler();

};

} } // namespace hyrise::taskscheduler

