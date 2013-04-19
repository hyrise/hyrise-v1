/*
 * AbstractCoreBoundQueuesScheduler.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "AbstractCoreBoundQueuesScheduler.h"

log4cxx::LoggerPtr AbstractCoreBoundQueuesScheduler::_logger = log4cxx::Logger::getLogger("taskscheduler.AbstractCoreBoundQueuesScheduler");


AbstractCoreBoundQueuesScheduler::AbstractCoreBoundQueuesScheduler(): _queues(0), _status(START_UP), _nextQueue(0) {
  // TODO Auto-generated constructor stub

}

AbstractCoreBoundQueuesScheduler::~AbstractCoreBoundQueuesScheduler() {
  // TODO Auto-generated destructor stub
}



AbstractCoreBoundQueuesScheduler::scheduler_status_t AbstractCoreBoundQueuesScheduler::getSchedulerStatus() {
  std::lock_guard<std::mutex> lk2(_statusMutex);
  return _status;
}
/*
 * schedule a given task
 */
void AbstractCoreBoundQueuesScheduler::schedule(std::shared_ptr<Task> task) {
  // simple strategy: check if task is ready to run -> then move to next taskqueue
  // otherwise store in wait list

  // lock the task - otherwise, a notify might happen prior to the task being added to the wait set
  task->lockForNotifications();
  if (task->isReady())
    pushToQueue(task);
  else {
    task->addReadyObserver(this);
    std::lock_guard<std::mutex> lk(_setMutex);
    _waitSet.insert(task);
    LOG4CXX_DEBUG(_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " inserted in wait queue");
  }
  task->unlockForNotifications();
}
/*
 * schedule a task for execution on a given core
 *
 */
void AbstractCoreBoundQueuesScheduler::schedule(std::shared_ptr<Task> task, int core) {
  task->setPreferredCore(core);
  schedule(task);
}

/* change the number of threads the task scheduler uses for running tasks;
 *
 */
void AbstractCoreBoundQueuesScheduler::resize(const size_t queues) {
  // set status to RESIZING
  // lock scheduler
  _statusMutex.lock();
  _status = RESIZING;
  _statusMutex.unlock();

  if (queues > _queues) {
    //set _queues to queues after new queues have been created to new tasks to be assigned to new queues
    // lock _queue mutex as queues are manipulated
    std::lock_guard<std::mutex> lk(_queuesMutex);
    if (static_cast<int>(queues) <= getNumberOfCoresOnSystem()) {
      for (size_t i = _queues; i < queues; ++i) {
        _taskQueues.push_back(createTaskQueue(i));
      }
      _queues = queues;
    } else {
      LOG4CXX_WARN(_logger, "number of queues exceeds available cores; set it to max available cores, which equals to " << std::to_string(getNumberOfCoresOnSystem()));
      resize(getNumberOfCoresOnSystem());
    }
  } else if (queues < _queues) {
    //set _queues to queues before queues have been deleted to avoid new tasks to be assigned to old queues
    // lock _queue mutex as queues are manipulated
    size_t queues_old;
    {
      std::lock_guard<std::mutex> lk(_queuesMutex);
      queues_old = _queues;
      _queues = queues;
      //adjust nextQueue to assign a task to in case it is larger than queue size;
      if (!(_nextQueue < queues))
        _nextQueue = 0;
    }
    for (size_t i = queues_old - 1; i > queues - 1; --i) {
      stopQueueAndRedistributeTasks(_taskQueues[i], queues);
      {
        std::lock_guard<std::mutex> lk(_queuesMutex);
        delete _taskQueues.back();
        _taskQueues.pop_back();
      }
    }
  }
  _statusMutex.lock();
  _status = RUN;
  _statusMutex.unlock();
}
/*
 * notify scheduler that a given task is ready
 */
void AbstractCoreBoundQueuesScheduler::notifyReady(std::shared_ptr<Task> task) {
  // remove task from wait set
  _setMutex.lock();
  int tmp = _waitSet.erase(task);
  _setMutex.unlock();

  // if task was found in wait set, schedule task to next queue
  if (tmp == 1) {
    LOG4CXX_DEBUG(_logger, "Task " << std::hex << (void *)task.get() << std::dec << " ready to run");
    pushToQueue(task);
  } else
    // should never happen, but check to identify potential race conditions
    LOG4CXX_ERROR(_logger, "Task that notified to be ready to run was not found / found more than once in waitSet! " << std::to_string(tmp));
}

/*
 * waits for all tasks to finish
 */
void AbstractCoreBoundQueuesScheduler::wait() {
  for (unsigned i = 0; i < _taskQueues.size(); ++i) {
    _taskQueues[i]->join();
  }
}

size_t AbstractCoreBoundQueuesScheduler::getNumberOfWorker() const {
  return _queues;
}

void AbstractCoreBoundQueuesScheduler::stopQueueAndRedistributeTasks(task_queue_t *queue, int queues) {
  std::vector<std::shared_ptr<Task> > tmp = queue->stopQueue();
  //redistribute tasks to other queues
  if (tmp.size() > 0) {
    int tmp_size = tmp.size();
    for (int i = 0; i < tmp_size; ++i) {
      // set preferred core to "NO_PREFERRED_CORE" (as queue with preferred core does not exist anymore / is used for other class of tasks)
      tmp.at(i)->setPreferredCore(NO_PREFERRED_CORE);
      pushToQueue(tmp.at(i));
    }
  }
}

void AbstractCoreBoundQueuesScheduler::shutdown() {
  _statusMutex.lock();
  _status = TO_STOP;
  _statusMutex.unlock();
  for (unsigned i = 0; i < _taskQueues.size(); ++i) {
    _taskQueues[i]->stopQueue();
  }
  _statusMutex.lock();
  _status = STOPPED;
  _statusMutex.unlock();
}
