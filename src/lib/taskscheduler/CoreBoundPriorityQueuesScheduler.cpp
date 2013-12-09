/*
 * CoreBoundPriorityQueuesScheduler.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "CoreBoundPriorityQueuesScheduler.h"
#include "helper/HwlocHelper.h"
#include "SharedScheduler.h"
#include <memory>

namespace hyrise {
namespace taskscheduler {

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<CoreBoundPriorityQueuesScheduler>("CoreBoundPriorityQueuesScheduler");
}

CoreBoundPriorityQueuesScheduler::CoreBoundPriorityQueuesScheduler(const int queues): AbstractCoreBoundQueuesScheduler(){
  _status = START_UP;
  // set _queues to queues after new queues have been created to new tasks to be assigned to new queues
  // lock _queue mutex as queues are manipulated
  std::lock_guard<lock_t> lk(_queuesMutex);
  if (queues <= getNumberOfCoresOnSystem()) {
    for (int i = 0; i < queues; ++i) {
      _taskQueues.push_back(createTaskQueue(i));
    }
    _queues = queues;
  } else {
    LOG4CXX_WARN(_logger, "number of queues exceeds available cores; set it to max available cores, which equals to " << std::to_string(getNumberOfCoresOnSystem()));
    for (int i = 0; i < getNumberOfCoresOnSystem(); ++i) {
      _taskQueues.push_back(createTaskQueue(i));
    }
    _queues = getNumberOfCoresOnSystem();
  }
  _status = RUN;
}

CoreBoundPriorityQueuesScheduler::~CoreBoundPriorityQueuesScheduler() {
  std::lock_guard<lock_t> lk2(this->_queuesMutex);
  task_queue_t *queue;
  for (unsigned i = 0; i < this->_taskQueues.size(); ++i) {
    queue =   this->_taskQueues[i];
    queue->stopQueue();
    delete queue;
  }
}

CoreBoundPriorityQueuesScheduler::task_queue_t *CoreBoundPriorityQueuesScheduler::createTaskQueue(int core) {
  return new CoreBoundPriorityQueue(core);
}

size_t CoreBoundPriorityQueuesScheduler::getNextQueue(){
  //round robin on cores
  std::lock_guard<lock_t> lk2(_queuesMutex);
  size_t next = _nextQueue;
  _nextQueue = (_nextQueue + 1) % _queues;
  return next;
}

void CoreBoundPriorityQueuesScheduler::pushToQueue(std::shared_ptr<Task> task) {
  count++;
  // check if task should be scheduled on specific core
  int core = task->getPreferredCore();
  if (core >= 0 && core < static_cast<int>(_queues)) {
    //potentially assigns task to a queue blocked by long running task
    _taskQueues[core]->push(task);
    LOG4CXX_DEBUG(this->_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " pushed to queue " << core);
  } else if (core == Task::NO_PREFERRED_CORE || core >= static_cast<int>(_queues)) {

    if (core < Task::NO_PREFERRED_CORE || core >= static_cast<int>(_queues))
      // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
      LOG4CXX_WARN(this->_logger, "Tried to assign task " << std::hex << (void *)task.get() << std::dec << " to core " << std::to_string(core) << " which is not assigned to scheduler; assigned it to next available core");
    
    size_t q = getNextQueue();      
    // simple strategy to avoid blocking of queues; check if queue is blocked - try a couple of times, otherwise schedule on next queue
    size_t retries = 0;
    while (static_cast<CoreBoundPriorityQueue *>(_taskQueues[q])->blocked() && retries < 100) {
      q = getNextQueue();
      ++retries;
    }
    this->_taskQueues[q]->push(task);
    //std::cout << "Task " <<  task->vname() << /*"; hex " << std::hex << &task << std::dec << */" pushed to queue " << _nextQueue << " count " << count << std::endl;
  }
}

} } // namespace hyrise::taskscheduler

