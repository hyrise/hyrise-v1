/*
 * WSCoreBoundPriorityQueue.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "WSCoreBoundPriorityQueuesScheduler.h"
#include "AbstractCoreBoundQueue.h"
#include "tbb/concurrent_priority_queue.h"

namespace hyrise {
namespace taskscheduler {

class WSCoreBoundPriorityQueuesScheduler;

class WSCoreBoundPriorityQueue : public AbstractCoreBoundQueue {

  typedef tbb::concurrent_priority_queue<std::shared_ptr<Task> , CompareTaskPtr> run_queue_t;
  run_queue_t _runQueue;
  WSCoreBoundPriorityQueuesScheduler * _scheduler;
  const std::vector<AbstractCoreBoundQueue *> * _allQueues;


private:
  std::shared_ptr<Task> stealTasks();

public:
  WSCoreBoundPriorityQueue(int core, WSCoreBoundPriorityQueuesScheduler *scheduler);
  virtual ~WSCoreBoundPriorityQueue();

  /*
   * Is executed by dedicated thread to work the queue
   */
  void executeTask();
  /*
   * push a new task to the queue, tasks are expected to have no unmet dependencies
   */
  void push(std::shared_ptr<Task> task);
  /*
   * stop queue and return remaining tasks; allows resizing the number of threads used by a task pool
   */
  std::vector<std::shared_ptr<Task> > stopQueue();

  /**
   * empty queue
   */
  std::vector<std::shared_ptr<Task> > emptyQueue();
  /*
   * steal Task
   * */
  std::shared_ptr<Task> stealTask();
  /*
   * set Task Queues
   */
  void refreshQueues();
};

} } // namespace hyrise::taskscheduler

