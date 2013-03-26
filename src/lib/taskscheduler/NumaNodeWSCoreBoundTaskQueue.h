/*
 * NumaNodeWSCoreBoundTaskQueue.h
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_NUMANODEWSCOREBOUNDTASKQUEUE_H_
#define SRC_LIB_TASKSCHEDULER_NUMANODEWSCOREBOUNDTASKQUEUE_H_

#include <taskscheduler/AbstractTaskQueue.h>
#include <taskscheduler/NumaNodeWSTaskScheduler.h>

template <class TaskQueue>
class NumaNodeWSTaskScheduler;

class NumaNodeWSCoreBoundTaskQueue : public AbstractCoreBoundTaskQueue {
  typedef std::deque<std::shared_ptr<Task> > run_queue_t;
  run_queue_t _runQueue;
  NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue> *_scheduler;
  unsigned _node;

private:
  std::shared_ptr<Task> stealTasks();

 public:

  /*
   * intializes a task queue and pins created thread to given core
   */
  NumaNodeWSCoreBoundTaskQueue(int core, NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue> *scheduler);
  virtual ~NumaNodeWSCoreBoundTaskQueue();
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
  NumaNodeWSCoreBoundTaskQueue::run_queue_t stopQueue();

  /**
   * empty queue
   */
  NumaNodeWSCoreBoundTaskQueue::run_queue_t emptyQueue();
  /*
   * steal Task
   * */
  std::shared_ptr<Task> stealTask();

  /*
   * intializes a task queue and pins created thread to given core
   */

};

#endif  // SRC_LIB_TASKSCHEDULER_NUMANODEWSCOREBOUNDTASKQUEUE_H_
