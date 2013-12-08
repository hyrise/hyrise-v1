/*
 * CoreBoundQueue.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#pragma once

#include "AbstractCoreBoundQueue.h"
#include <vector>
#include <queue>

namespace hyrise {
namespace taskscheduler {

/*
 * A queue with a dedicated worker thread; used by SimpleTaskScheduler to run tasks
 */
class CoreBoundQueue : public AbstractCoreBoundQueue {
  std::queue<std::shared_ptr<Task> > _runQueue;
  bool _blocked;

 public:
  /*
   * intializes a task queue and pins created thread to given core
   */
  CoreBoundQueue(int core);
  ~CoreBoundQueue();
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
   * check whether queue is blocked / queue is blocked if it is currently executing a task
   */
  bool blocked() {
    return _blocked;
  }
};

} } // namespace hyrise::taskscheduler

