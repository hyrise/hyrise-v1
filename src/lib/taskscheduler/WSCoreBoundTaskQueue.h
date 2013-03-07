/*
 * WSCoreBoundTaskQueue.h
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_WSCOREBOUNDTASKQUEUE_H_
#define SRC_LIB_TASKSCHEDULER_WSCOREBOUNDTASKQUEUE_H_

#include <taskscheduler/AbstractTaskQueue.h>
#include <taskscheduler/WSSimpleTaskScheduler.h>

template <class TaskQueue>
class WSSimpleTaskScheduler;

/*
 * A queue with a dedicated worker thread and work stealing; used by SimpleTaskScheduler to run tasks
 * uses double ended queue as tasks to run are taken from front, tasks to steal from back
 */
class WSCoreBoundTaskQueue : public AbstractCoreBoundTaskQueue {
  typedef std::deque<std::shared_ptr<Task> > run_queue_t;
  run_queue_t _runQueue;
  WSSimpleTaskScheduler<WSCoreBoundTaskQueue> *_scheduler;

 private:
  std::shared_ptr<Task> stealTasks();

 public:

  /*
   * intializes a task queue and pins created thread to given core
   */
  WSCoreBoundTaskQueue(int core, WSSimpleTaskScheduler<WSCoreBoundTaskQueue> *scheduler);
  ~WSCoreBoundTaskQueue();
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
  WSCoreBoundTaskQueue::run_queue_t stopQueue();

  /**
   * empty queue
   */
  WSCoreBoundTaskQueue::run_queue_t emptyQueue();
  /*
   * steal Task
   * */
  std::shared_ptr<Task> stealTask();
};

#endif  // SRC_LIB_TASKSCHEDULER_WSCOREBOUNDTASKQUEUE_H_
