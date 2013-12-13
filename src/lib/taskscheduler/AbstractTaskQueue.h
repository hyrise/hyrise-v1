// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * AbstractTaskQueue.h
 *
 *  Created on: Jun 6, 2012
 *      Author: jwust
 */

#pragma once

#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>
#include <log4cxx/logger.h>
#include <taskscheduler/Task.h>

#include "helper/locking.h"

namespace hyrise {
namespace taskscheduler {

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

  typedef hyrise::locking::Spinlock lock_t;

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

} } // namespace hyrise::taskscheduler

