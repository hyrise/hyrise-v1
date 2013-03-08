#ifndef SRC_LIB_TASKSCHEDULER_WSSIMPLETASKSCHEDULER_H_
#define SRC_LIB_TASKSCHEDULER_WSSIMPLETASKSCHEDULER_H_

#include <taskscheduler/AbstractTaskScheduler.h>
#include <taskscheduler/WSCoreBoundTaskQueue.h>
#include <deque>
#include <iostream>
#include "helper/HwlocHelper.h"

template<class TaskQueue>
class WSSimpleTaskScheduler : public AbstractQueueBasedTaskScheduler<TaskQueue> {
protected:


  static bool registered;

public:
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::task_queues_t task_queues_t;
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::task_queue_t task_queue_t;

  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::scheduler_status_t scheduler_status_t;

  WSSimpleTaskScheduler(const int queues = getNumberOfCoresOnSystem()): AbstractQueueBasedTaskScheduler<TaskQueue>() {
    // call resizeQueues here and not in AbstractQueueBasedTaskScheduler, as resizeQueue calls virtual createTaskQueue
    this->resize(queues);
  }

  virtual ~WSSimpleTaskScheduler() {
    this->_statusMutex.lock();
    this->_status = AbstractQueueBasedTaskScheduler<TaskQueue>::TO_STOP;
    this->_statusMutex.unlock();
    task_queue_t *queue;
    for (size_t i = 0; i < this->_queues; ++i) {
      queue =   this->_taskQueues[i];
      queue->stopQueue();
      delete queue;
    }
  }

  const task_queues_t *getTaskQueues() {
    // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return NULL
    {
      std::lock_guard<std::mutex> lk2(this->_statusMutex);
      if (this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::RESIZING
          || this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::TO_STOP
          || this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::STOPPED)
        return NULL;
    }
    // return const reference to task queues
    std::lock_guard<std::mutex> lk(this->_queuesMutex);
    return &this->_taskQueues;
  }

protected:
  void pushToQueue(std::shared_ptr<Task> task) {
    int core = task->getPreferredCore();
    // lock queueMutex to push task to queue
    std::lock_guard<std::mutex> lk2(this->_queuesMutex);
    if (core >= 0 && core < static_cast<int>(this->_queues)) {
      // push task to queue that runs on given core
      this->_taskQueues[core]->push(task);
      LOG4CXX_DEBUG(this->_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " pushed to queue " << core);
    } else if (core == NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues)) {
      if (core < NO_PREFERRED_CORE || core >= static_cast<int>(this->_queues))
        // Tried to assign task to core which is not assigned to scheduler; assigned to other core, log warning
        LOG4CXX_WARN(this->_logger, "Tried to assign task " << std::hex << (void *)task.get() << std::dec << " to core " << std::to_string(core) << " which is not assigned to scheduler; assigned it to next available core");
      // push task to next queue
      this->_taskQueues[this->_nextQueue]->push(task);
      //std::cout << "Task " <<  task->vname() << "; hex " << std::hex << &task << std::dec << " pushed to queue " << this->_nextQueue << std::endl;
      //round robin on cores
      this->_nextQueue = (this->_nextQueue + 1) % this->_queues;
    }
  }

  void stopQueueAndRedistributeTasks(task_queue_t *queue, int queues) {
    std::deque<std::shared_ptr<Task> > tmp = queue->stopQueue();
    //redistribute tasks to other queues
    if (tmp.size() > 0) {
      int tmp_size = tmp.size();
      {
        for (int i = 0; i < tmp_size; ++i) {
          // set preferred core to "NO_PREFERRED_CORE"
          // set preferred core to "NO_PREFERRED_CORE" (as queue with preferred core does not exist anymore / is used for other class of tasks)
          tmp.front()->setPreferredCore(NO_PREFERRED_CORE);
          pushToQueue(tmp.front());
          tmp.pop_front();
        }
      }
    }
  }
  virtual task_queue_t *createTaskQueue(int core) {
    return new task_queue_t(core, this);
  }
};

#endif  // SRC_LIB_TASKSCHEDULER_WSSIMPLETASKSCHEDULER_H_
