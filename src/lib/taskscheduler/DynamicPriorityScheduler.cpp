// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "DynamicPriorityScheduler.h"
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

// register Scheduler at SharedScheduler
namespace {
  bool registered = hyrise::taskscheduler::SharedScheduler::registerScheduler<DynamicPriorityScheduler>("DynamicPriorityScheduler");
  log4cxx::LoggerPtr _logger = log4cxx::Logger::getLogger("taskscheduler.DynamicPriorityScheduler");
}

DynamicPriorityScheduler::DynamicPriorityScheduler(int threads):CentralPriorityScheduler(threads){}

void DynamicPriorityScheduler::schedule(std::shared_ptr<Task> task){
  if (task->isDynamic() && task->isReady()) {
    uint dynamicCount = task->determineDynamicCount(_maxTaskSize);
    auto tasks = task->applyDynamicParallelization(dynamicCount);
    for (const auto& i : tasks) {
      CentralPriorityScheduler::schedule(i);
    }
  } else {
    CentralPriorityScheduler::schedule(task);
  }
}

void DynamicPriorityScheduler::notifyReady(std::shared_ptr<Task> task){
	// remove task from wait set
  _setMutex.lock();
  int tmp = _waitSet.erase(task);
  _setMutex.unlock();

  // if task was found in wait set, schedule task to next queue
  if (tmp == 1) {
    LOG4CXX_DEBUG(_logger, "Task " << std::hex << (void *)task.get() << std::dec << " ready to run");
    if (task->isDynamic()) {
      auto dynamicCount = task->determineDynamicCount(_maxTaskSize);
      auto tasks = task->applyDynamicParallelization(dynamicCount);
      for (const auto& i : tasks) {
        if (i->isReady()) {
          std::lock_guard<decltype(_queueMutex)> lk(_queueMutex);
          _runQueue.push(i);
          _condition.notify_one();
        } else {   
          i->addReadyObserver(shared_from_this());
          std::lock_guard<decltype(_setMutex)> lk(_setMutex);
          _waitSet.insert(i);
          LOG4CXX_DEBUG(_logger,  "Task " << std::hex << (void *)i.get() << std::dec << " inserted in wait queue");
        }
      }
    } else { // task is not dynamic
      std::lock_guard<decltype(_queueMutex)> lk(_queueMutex);
      _runQueue.push(task);
      _condition.notify_one();
    }
  } else {
    // should never happen, but check to identify potential race conditions
    LOG4CXX_ERROR(_logger, "Task that notified to be ready to run was not found / found more than once in waitSet! " << std::to_string(tmp));
  }    
}
}}
