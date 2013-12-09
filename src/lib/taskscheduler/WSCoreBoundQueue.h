/*
 * WSCoreBoundQueue.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include <deque>
#include "WSCoreBoundQueuesScheduler.h"
#include "AbstractCoreBoundQueue.h"

namespace hyrise {
namespace taskscheduler {

class WSCoreBoundQueuesScheduler;

class WSCoreBoundQueue : public AbstractCoreBoundQueue {

  typedef std::deque<std::shared_ptr<Task> > run_queue_t;
  run_queue_t _runQueue;
  WSCoreBoundQueuesScheduler * _scheduler;

private:
  std::shared_ptr<Task> stealTasks();

public:
  WSCoreBoundQueue(int core, WSCoreBoundQueuesScheduler *scheduler);
  virtual ~WSCoreBoundQueue();

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
};

} } // namespace hyrise::taskscheduler

