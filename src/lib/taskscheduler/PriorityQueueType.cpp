// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "PriorityQueueType.h"
namespace hyrise {
namespace taskscheduler {
void PriorityQueueType::push(const std::shared_ptr<Task>& task) { _runQueue.push(task); }
bool PriorityQueueType::try_pop(std::shared_ptr<Task>& task) { return _runQueue.try_pop(task); }
size_t PriorityQueueType::unsafe_size() { return _runQueue.size(); }
size_t PriorityQueueType::size() { NOT_IMPLEMENTED }
}
}