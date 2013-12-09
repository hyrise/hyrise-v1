/*
 * WSCoreBoundQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "AbstractCoreBoundQueuesScheduler.h"
#include "AbstractCoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

class WSCoreBoundQueuesScheduler : public AbstractCoreBoundQueuesScheduler {

  /**
   * push ready task to the next queue
   */
  virtual void pushToQueue(std::shared_ptr<Task> task);

  /*
   * create a new task queue
   */
  virtual WSCoreBoundQueuesScheduler::task_queue_t *createTaskQueue(int core);


public:
  WSCoreBoundQueuesScheduler(int queues = getNumberOfCoresOnSystem());
  virtual ~WSCoreBoundQueuesScheduler();

  const std::vector<AbstractCoreBoundQueue *> *getTaskQueues();

};

} } // namespace hyrise::taskscheduler

