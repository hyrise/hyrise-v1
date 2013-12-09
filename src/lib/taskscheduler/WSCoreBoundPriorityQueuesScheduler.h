/*
 * WSCoreBoundPriorityQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "AbstractCoreBoundQueuesScheduler.h"
#include "AbstractCoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

class WSCoreBoundPriorityQueuesScheduler : public AbstractCoreBoundQueuesScheduler {

  /**
   * push ready task to the next queue
   */
  virtual void pushToQueue(std::shared_ptr<Task> task);

  /*
   * create a new task queue
   */
  virtual WSCoreBoundPriorityQueuesScheduler::task_queue_t *createTaskQueue(int core);


public:
  WSCoreBoundPriorityQueuesScheduler(int queues = getNumberOfCoresOnSystem());
  virtual ~WSCoreBoundPriorityQueuesScheduler();

  const std::vector<AbstractCoreBoundQueue *> *getTaskQueues();
};

} } // namespace hyrise::taskscheduler

