// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * AbstractTaskScheduler.h
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#pragma once

/*
 * Base class for Task Schedulers
 *
 */

#include "taskscheduler/Task.h"
#include "taskscheduler/AbstractTaskQueue.h"
#include <memory>
#include <unordered_set>
#include <iostream>
#include <log4cxx/logger.h>
#include "helper/HwlocHelper.h"
#include "helper/locking.h"

namespace hyrise {
namespace taskscheduler {

class AbstractTaskScheduler {
  /*
   * definition of scheduler status
   */
 public:
  typedef enum {
    START_UP = -1,
    RUN = 0,
    RESIZING = 1,
    TO_STOP = 2,
    STOPPED = 3
  } scheduler_state;
  typedef int scheduler_status_t; 

  typedef hyrise::locking::Spinlock lock_t;

  virtual ~AbstractTaskScheduler() {};
  /*
   * schedule a task for execution
   */
  virtual void schedule(std::shared_ptr<Task> task) = 0;
  /*
   * schedule a list of tasks belonging to a query for execution
   */
  virtual void scheduleQuery(std::vector<std::shared_ptr<Task> > tasks){
    for (const auto& task: tasks) {
      schedule(task);
    }
  }
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown() = 0;
  /**
   * get number of worker
   */
  virtual size_t getNumberOfWorker() const = 0;
};

} } // namespace hyrise::taskscheduler

