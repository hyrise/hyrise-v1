/*
 * CoreBoundTaskQueue.h
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_COREBOUNDTASKQUEUE_H_
#define SRC_LIB_TASKSCHEDULER_COREBOUNDTASKQUEUE_H_

#include <thread>
#include <queue>
#include <taskscheduler/AbstractTaskQueue.h>


/*
 * A queue with a dedicated worker thread; used by SimpleTaskScheduler to run tasks
 */
class CoreBoundTaskQueue : public AbstractCoreBoundTaskQueue {
  std::queue<std::shared_ptr<Task> > _runQueue;
  bool _blocked;

 public:
  /*
   * intializes a task queue and pins created thread to given core
   */
  CoreBoundTaskQueue(int core);
  ~CoreBoundTaskQueue();
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
  std::queue<std::shared_ptr<Task> > stopQueue();

  /**
   * empty queue
   */
  std::queue<std::shared_ptr<Task> > emptyQueue();
  /*
   * check whether queue is blocked / queue is blocked if it is currently executing a task
   */
  bool blocked() {
    return _blocked;
  }
};

#endif  // SRC_LIB_TASKSCHEDULER_COREBOUNDTASKQUEUE_H_
