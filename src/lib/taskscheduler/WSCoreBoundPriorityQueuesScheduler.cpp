/*
 * WSCoreBoundPriorityQueuesScheduler.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "WSCoreBoundPriorityQueuesScheduler.h"
#include "WSCoreBoundPriorityQueue.h"
#include "SharedScheduler.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<WSCoreBoundPriorityQueuesScheduler>("WSCoreBoundPriorityQueuesScheduler");
}

WSCoreBoundPriorityQueuesScheduler::WSCoreBoundPriorityQueuesScheduler(const int queues) : AbstractCoreBoundQueuesScheduler(){
  // call resizeQueues here and not in AbstractQueueBasedTaskScheduler, as resizeQueue calls virtual createTaskQueue
  this->resize(queues);
  for (unsigned i = 0; i < _taskQueues.size(); ++i) {
    static_cast<WSCoreBoundPriorityQueue *>(_taskQueues[i])->refreshQueues();
  }

}

WSCoreBoundPriorityQueuesScheduler::~WSCoreBoundPriorityQueuesScheduler() {
  this->_status = AbstractCoreBoundQueuesScheduler::TO_STOP;
  task_queue_t *queue;
  for (size_t i = 0; i < this->_queues; ++i) {
    queue =   this->_taskQueues[i];
    queue->stopQueue();
    delete queue;
  }
}

const std::vector<AbstractCoreBoundQueue *> *WSCoreBoundPriorityQueuesScheduler::getTaskQueues() {
   // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return NULL
  if (this->_status == AbstractCoreBoundQueuesScheduler::RESIZING
      || this->_status == AbstractCoreBoundQueuesScheduler::TO_STOP
      || this->_status == AbstractCoreBoundQueuesScheduler::STOPPED)
    return NULL;
  // return const reference to task queues
  std::lock_guard<std::mutex> lk(this->_queuesMutex);
  return &this->_taskQueues;
 }

void WSCoreBoundPriorityQueuesScheduler::pushToQueue(std::shared_ptr<Task> task) {
    int core = task->getPreferredCore();
    // lock queueMutex to push task to queue
    if (core >= 0 && core < static_cast<int>(this->_queues)) {
      // push task to queue that runs on given core
      this->_taskQueues[core]->push(task);
      LOG4CXX_DEBUG(this->_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " pushed to queue " << core);
    } else if (core == NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues)) {
      if (core < NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues))
        // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
        LOG4CXX_WARN(this->_logger, "Tried to assign task " << std::hex << (void *)task.get() << std::dec << " to core " << std::to_string(core) << " which is not assigned to scheduler; assigned it to next available core");
      // push task to next queue
      {
        std::lock_guard<std::mutex> lk2(this->_queuesMutex);
        this->_taskQueues[this->_nextQueue]->push(task);
        //std::cout << "Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " pushed to queue " << this->_nextQueue << std::endl;
        //round robin on cores
        this->_nextQueue = (this->_nextQueue + 1) % this->_queues;
      }
    }
  }

WSCoreBoundPriorityQueuesScheduler::task_queue_t *WSCoreBoundPriorityQueuesScheduler::createTaskQueue(int core) {
    return new WSCoreBoundPriorityQueue(core, this);
  }

void WSCoreBoundPriorityQueuesScheduler::resize(const size_t queues) {
  // set status to RESIZING
  _status = RESIZING;
  // shutdown and restart all queues - fast dynamic resizing is currently not in scope
  if(_queues > 0){
    task_queue_t *queue;
    for (size_t i = 0; i < this->_queues; ++i) {
       queue = this->_taskQueues[i];
       queue->stopQueue();
       delete queue;
    }
  }

  std::lock_guard<std::mutex> lk(_queuesMutex);
  if (static_cast<int>(queues) <= getNumberOfCoresOnSystem()) {
    for (size_t i = 0; i < queues; ++i) {
      _taskQueues.push_back(createTaskQueue(i));
    }
    _queues = queues;
  } else {
    LOG4CXX_WARN(_logger, "number of queues exceeds available cores; set it to max available cores, which equals to " << std::to_string(getNumberOfCoresOnSystem()));
    resize(getNumberOfCoresOnSystem());
  }
  for (unsigned i = 0; i < _taskQueues.size(); ++i) {
    static_cast<WSCoreBoundPriorityQueue *>(_taskQueues[i])->refreshQueues();
  }
  _status = RUN;
}
