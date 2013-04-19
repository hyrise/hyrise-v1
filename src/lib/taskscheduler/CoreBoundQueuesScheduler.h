/*
 * CoreBoundQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#ifndef COREBOUNDQUEUESSCHEDULER_H_
#define COREBOUNDQUEUESSCHEDULER_H_

#include "taskscheduler/Task.h"
#include "taskscheduler/CoreBoundQueue.h"
#include "taskscheduler/AbstractCoreBoundQueuesScheduler.h"
#include <memory>
#include <unordered_set>
#include <log4cxx/logger.h>


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

#endif /* COREBOUNDQUEUESSCHEDULER_H_ */

