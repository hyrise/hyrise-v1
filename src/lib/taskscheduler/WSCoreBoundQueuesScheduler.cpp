/*
 * WSCoreBoundQueuesScheduler.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "WSCoreBoundQueuesScheduler.h"
#include "WSCoreBoundQueue.h"
#include "SharedScheduler.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<WSCoreBoundQueuesScheduler>("WSCoreBoundQueuesScheduler");
}

WSCoreBoundQueuesScheduler::WSCoreBoundQueuesScheduler(const int queues) : AbstractCoreBoundQueuesScheduler(){
  // call resizeQueues here and not in AbstractQueueBasedTaskScheduler, as resizeQueue calls virtual createTaskQueue
  this->resize(queues);

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
   // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return NULL

     if (this->_status == AbstractCoreBoundQueuesScheduler::RESIZING
         || this->_status == AbstractCoreBoundQueuesScheduler::TO_STOP
         || this->_status == AbstractCoreBoundQueuesScheduler::STOPPED)
       return NULL;
   // return const reference to task queues
   std::lock_guard<std::mutex> lk(this->_queuesMutex);
   return &this->_taskQueues;
 }

void WSCoreBoundQueuesScheduler::pushToQueue(std::shared_ptr<Task> task) {
    int core = task->getPreferredCore();
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
        std::cout << "Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " pushed to queue " << this->_nextQueue << std::endl;
        //round robin on cores
        this->_nextQueue = (this->_nextQueue + 1) % this->_queues;
      }
    }
  }

  WSCoreBoundQueuesScheduler::task_queue_t *WSCoreBoundQueuesScheduler::createTaskQueue(int core) {
    return new WSCoreBoundQueue(core, this);
  }
