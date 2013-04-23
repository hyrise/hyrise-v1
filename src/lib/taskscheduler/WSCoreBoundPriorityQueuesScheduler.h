/*
 * WSCoreBoundPriorityQueuesScheduler.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#ifndef WSCOREBOUNDPRIORITYQUEUESSCHEDULER_H_
#define WSCOREBOUNDPRIORITYQUEUESSCHEDULER_H_

#include "AbstractCoreBoundQueuesScheduler.h"
#include "AbstractCoreBoundQueue.h"

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

  void resize(const size_t queues);

};


#endif /* WSCOREBOUNDPRIORITYQUEUESSCHEDULER_H_ */
