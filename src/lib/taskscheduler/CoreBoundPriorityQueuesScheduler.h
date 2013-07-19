/*
 * CoreBoundPriorityQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#ifndef COREBOUNDPRIORITYQUEUESSCHEDULER_H_
#define COREBOUNDPRIORITYQUEUESSCHEDULER_H_

#include "taskscheduler/Task.h"
#include "taskscheduler/CoreBoundPriorityQueue.h"
#include "taskscheduler/AbstractCoreBoundQueuesScheduler.h"
#include <memory>
#include <unordered_set>
#include <log4cxx/logger.h>


/**
 * a task scheduler with thread specific queues
 */
class CoreBoundPriorityQueuesScheduler : public AbstractCoreBoundQueuesScheduler {

   /**
    * push ready task to the next queue
    */
   virtual void pushToQueue(std::shared_ptr<Task> task);

   /*
    * create a new task queue
    */
   virtual task_queue_t *createTaskQueue(int core);

public:
  CoreBoundPriorityQueuesScheduler(int queues = getNumberOfCoresOnSystem());
  virtual ~CoreBoundPriorityQueuesScheduler();

};

#endif /* COREBOUNDPRIORITYQUEUESSCHEDULER_H_ */

