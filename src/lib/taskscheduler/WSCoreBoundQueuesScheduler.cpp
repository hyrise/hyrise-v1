/*
 * WSCoreBoundQueuesScheduler.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "WSCoreBoundQueuesScheduler.h"
#include "WSCoreBoundQueue.h"
#include "SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<WSCoreBoundQueuesScheduler>("WSCoreBoundQueuesScheduler");
}

WSCoreBoundQueuesScheduler::WSCoreBoundQueuesScheduler(const int queues) : AbstractCoreBoundQueuesScheduler(){
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

WSCoreBoundQueuesScheduler::~WSCoreBoundQueuesScheduler() {
  this->_status = AbstractCoreBoundQueuesScheduler::TO_STOP;
  task_queue_t *queue;
  for (size_t i = 0; i < this->_queues; ++i) {
    queue =   this->_taskQueues[i];
    queue->stopQueue();
    delete queue;
  }
}

const std::vector<AbstractCoreBoundQueue *> *WSCoreBoundQueuesScheduler::getTaskQueues() {
   // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return nullptr

     if (this->_status == AbstractCoreBoundQueuesScheduler::RESIZING
         || this->_status == AbstractCoreBoundQueuesScheduler::TO_STOP
         || this->_status == AbstractCoreBoundQueuesScheduler::STOPPED)
       return nullptr;
   // return const reference to task queues
   std::lock_guard<lock_t> lk(this->_queuesMutex);
   return &this->_taskQueues;
 }

void WSCoreBoundQueuesScheduler::pushToQueue(std::shared_ptr<Task> task) {
    int core = task->getPreferredCore();
    if (core >= 0 && core < static_cast<int>(this->_queues)) {
      // push task to queue that runs on given core
      this->_taskQueues[core]->push(task);
      LOG4CXX_DEBUG(this->_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " pushed to queue " << core);
    } else if (core == Task::NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues)) {
      if (core < Task::NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues))
        // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
        LOG4CXX_WARN(this->_logger, "Tried to assign task " << std::hex << (void *)task.get() << std::dec << " to core " << std::to_string(core) << " which is not assigned to scheduler; assigned it to next available core");
      // push task to next queue
      {
        std::lock_guard<lock_t> lk2(this->_queuesMutex);
        this->_taskQueues[this->_nextQueue]->push(task);
        //std::cout << "Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " pushed to queue " << this->_nextQueue << std::endl;
        //round robin on cores
        this->_nextQueue = (this->_nextQueue + 1) % this->_queues;
      }
    }
  }

WSCoreBoundQueuesScheduler::task_queue_t *WSCoreBoundQueuesScheduler::createTaskQueue(int core) {
  return new WSCoreBoundQueue(core, this);
}

} } // namespace hyrise::taskscheduler

