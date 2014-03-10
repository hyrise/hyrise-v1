// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include <tbb/concurrent_priority_queue.h>
#include <memory>
#include "helper/not_implemented.h"
#include "Task.h"

namespace hyrise {
namespace taskscheduler {

/*
* A wrapper class for a queue type which is used by the Scheduler Classes
* The motivation was to provide a common interface to tbb concurrent priority queue
* and tbb concurrent queue.
* A new queue can be easily implemented by implementing this interface; all Scheduler
* classes are templated with this Queue
*/

class AbstractQueueType {
 public:
  virtual void push(const std::shared_ptr<Task>& task) = 0;
  virtual bool try_pop(std::shared_ptr<Task>& task) = 0;
  virtual size_t unsafe_size() = 0;
  // currently not used
  virtual size_t size() = 0;
};
}
}
