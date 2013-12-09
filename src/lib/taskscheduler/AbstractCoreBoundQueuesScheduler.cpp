/*
 * AbstractCoreBoundQueuesScheduler.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#include "AbstractCoreBoundQueuesScheduler.h"

namespace hyrise {
namespace taskscheduler {

log4cxx::LoggerPtr AbstractCoreBoundQueuesScheduler::_logger = log4cxx::Logger::getLogger("taskscheduler.AbstractCoreBoundQueuesScheduler");


AbstractCoreBoundQueuesScheduler::AbstractCoreBoundQueuesScheduler(): _queues(0), _status(START_UP), _nextQueue(0) {
}

AbstractCoreBoundQueuesScheduler::~AbstractCoreBoundQueuesScheduler() {
  // TODO Auto-generated destructor stub
}

AbstractCoreBoundQueuesScheduler::scheduler_status_t AbstractCoreBoundQueuesScheduler::getSchedulerStatus() {
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
    task->addReadyObserver(shared_from_this());
    std::lock_guard<lock_t> lk(_setMutex);
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


void AbstractCoreBoundQueuesScheduler::shutdown() {
  _status = TO_STOP;
  for (unsigned i = 0; i < _taskQueues.size(); ++i) {
    _taskQueues[i]->stopQueue();
  }
  _status = STOPPED;
}

} } // namespace hyrise::taskscheduler

