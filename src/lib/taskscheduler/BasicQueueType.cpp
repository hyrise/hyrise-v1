// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "BasicQueueType.h"
namespace hyrise {
namespace taskscheduler {
void BasicQueueType::push(const std::shared_ptr<Task>& task) { _runQueue.push(task); }
bool BasicQueueType::try_pop(std::shared_ptr<Task>& task) { return _runQueue.try_pop(task); }
size_t BasicQueueType::unsafe_size() { return _runQueue.unsafe_size(); }
size_t BasicQueueType::size() { NOT_IMPLEMENTED }
}
}
