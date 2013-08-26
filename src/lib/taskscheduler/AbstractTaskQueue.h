// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * AbstractTaskQueue.h
 *
 *  Created on: Jun 6, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_ABSTRACTTASKQUEUE_H_
#define SRC_LIB_TASKSCHEDULER_ABSTRACTTASKQUEUE_H_

#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <log4cxx/logger.h>
#include <taskscheduler/Task.h>

class AbstractTaskQueue {

 public:
  typedef enum {
    STARTUP = 0,
    RUN = 1,
    RUN_UNTIL_DONE = 2,
    TO_STOP = 3,
    STOPPED = 4,
  } queue_state;
  typedef int queue_status_t;

  virtual ~AbstractTaskQueue() {};

  virtual void executeTask() = 0;
  /*
   * push a new task to the queue, tasks are expected to have no unmet dependencies
   */
  virtual void push(std::shared_ptr<Task> task) = 0;
  /*
   * wait until all tasks are done
   */
  virtual void join() = 0;
};

#endif  // SRC_LIB_TASKSCHEDULER_ABSTRACTTASKQUEUE_H_
