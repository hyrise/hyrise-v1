/*
 * TaskQueue.h
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_TASKQUEUE_H_
#define SRC_LIB_TASKSCHEDULER_TASKQUEUE_H_

#include "taskscheduler/Task.h"
#include "taskscheduler/TaskQueue.h"
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <log4cxx/logger.h>

enum status {
  RUN = 0,
  RUN_UNTIL_DONE = 1,
  TO_STOP = 2,
  STOPPED = 3
};

/*
 * A queue with a dedicated worker thread; used by SimpleTaskScheduler to run tasks
 */
class TaskQueue {
  std::queue<std::shared_ptr<Task> > _runQueue;
  std::thread *_thread;
  //the core the thread should run on
  int _core;
  bool _blocked;
  status _status;
  std::mutex _queueMutex;
  std::mutex _threadStatusMutex;
  std::condition_variable _condition;

  void launchThread(int core);

 public:
  static log4cxx::LoggerPtr logger;
  /*
   * intializes a task queue and pins created thread to given core
   */
  TaskQueue(int core);
  ~TaskQueue();
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
  std::queue<std::shared_ptr<Task> > *stopQueue();
  /*
   * wait until all tasks are done
   */
  void join();
  //get statistics
  //steal task (with priority)

  bool blocked() {
    return _blocked;
  }
};

#endif  // SRC_LIB_TASKSCHEDULER_TASKQUEUE_H_
