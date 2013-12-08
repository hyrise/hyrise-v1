/*
 * ThreadPerTaskScheduler.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: jwust
 */

#include "ThreadPerTaskScheduler.h"

namespace hyrise {
namespace taskscheduler {

log4cxx::LoggerPtr ThreadPerTaskScheduler::_logger = log4cxx::Logger::getLogger("taskscheduler.ThreadPerTaskScheduler");

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<ThreadPerTaskScheduler>("ThreadPerTaskScheduler");
}

ThreadPerTaskScheduler::ThreadPerTaskScheduler() {
  _status = RUN;
}
ThreadPerTaskScheduler::ThreadPerTaskScheduler(int i) {
  _status = RUN;
}

ThreadPerTaskScheduler::~ThreadPerTaskScheduler() {
  // TODO Auto-generated destructor stub
}

/*
 * schedule a task for execution
 */
void ThreadPerTaskScheduler::schedule(std::shared_ptr<Task> task){
  // simple strategy: check if task is ready to run -> create new thread and run
  // otherwise store in wait list

  // lock the task - otherwise, a notify might happen prior to the task being added to the wait set
  task->lockForNotifications();
  //std::cout << "scheduled task " << task->vname() <<std::endl;

  if (task->isReady()){
    //std::cout << "start thread with task " << task->vname() <<std::endl;
    std::thread t((TaskExecutor(task)));
    t.detach();
  }
  else {
    task->addReadyObserver(shared_from_this());
    std::lock_guard<lock_t> lk(_setMutex);
    _waitSet.insert(task);
    LOG4CXX_DEBUG(_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " inserted in wait queue");
  }
  task->unlockForNotifications();
}
/*
 * shutdown task scheduler; makes sure all underlying threads are stopped
 */
void ThreadPerTaskScheduler::shutdown(){
}

/*
 * notify scheduler that a given task is ready
 */
void ThreadPerTaskScheduler::notifyReady(std::shared_ptr<Task> task) {
  // remove task from wait set
  _setMutex.lock();
  int tmp = _waitSet.erase(task);
  _setMutex.unlock();
  // if task was found in wait set, schedule task
  if (tmp == 1) {
    std::thread t((TaskExecutor(task)));
    t.detach();
    //std::cout << "task ready: " << task->vname() <<std::endl;
    //std::thread *t = new std::thread(TaskExecutor(task));
    //t->detach();
  } else {
    //std::cout << "task ready fail: " << task->vname() <<std::endl;
    // should never happen, but check to identify potential race conditions
    LOG4CXX_ERROR(_logger, "Task that notified to be ready to run was not found / found more than once in waitSet! " << std::to_string(tmp));
  }
}

} } // namespace hyrise::taskscheduler

