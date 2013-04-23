// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * AbstractTaskScheduler.h
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_ABSTRACTTASKSCHEDULER_H_
#define SRC_LIB_TASKSCHEDULER_ABSTRACTTASKSCHEDULER_H_

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

class AbstractTaskScheduler {
  /*
   * definition of scheduler status, can be used to sync actions in queue, like work stealing, with scheduler status (e.g. to avoid stealing tasks from queue, while resizing)
   */
 public:
  typedef enum status {
    START_UP = -1,
    RUN = 0,
    RESIZING = 1,
    TO_STOP = 2,
    STOPPED = 3
  } scheduler_status_t;

  virtual ~AbstractTaskScheduler() {};
  /*
   * schedule a task for execution
   */
  virtual void schedule(std::shared_ptr<Task> task) = 0;
  /*
   * shutdown task scheduler; makes sure all underlying threads are stopped
   */
  virtual void shutdown() = 0;
  /*
   * resize the number of worker threads/queues
   */
  virtual void resize(const size_t queues) = 0;
  /**
   * get number of worker
   */
  virtual size_t getNumberOfWorker() const = 0;
};

#endif  // SRC_LIB_TASKSCHEDULER_ABSTRACTTASKSCHEDULER_H_
