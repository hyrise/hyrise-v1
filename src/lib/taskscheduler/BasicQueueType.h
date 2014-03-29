// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include <tbb/concurrent_queue.h>
#include "AbstractQueueType.h"

namespace hyrise {
namespace taskscheduler {

class BasicQueueType : public AbstractQueueType {
  tbb::concurrent_queue<std::shared_ptr<Task>> _runQueue;

 public:
  void push(const std::shared_ptr<Task>& task);
  bool try_pop(std::shared_ptr<Task>& task);
  size_t unsafe_size();
  size_t size();
};
}
}
