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
bool registered = SharedScheduler::registerScheduler<ThreadPerTaskScheduler>("ThreadPerTaskScheduler");
}

ThreadPerTaskScheduler::ThreadPerTaskScheduler() { _status = RUN; }

void ThreadPerTaskScheduler::init() {}


ThreadPerTaskScheduler::ThreadPerTaskScheduler(int i) { _status = RUN; }

ThreadPerTaskScheduler::~ThreadPerTaskScheduler() {
  // TODO Auto-generated destructor stub
}

/*
 * schedule a task for execution
 */
void ThreadPerTaskScheduler::schedule(const std::shared_ptr<Task>& task) {
  // simple strategy: check if task is ready to run -> create new thread and run
  // otherwise store in wait list

  // lock the task - otherwise, a notify might happen prior to the task being added to the wait set
  task->lockForNotifications();
  if (task->isReady()) {
    task->unlockForNotifications();
    std::thread t((TaskExecutor(task)));
    t.detach();
  } else {
    task->addReadyObserver(shared_from_this());
    task->unlockForNotifications();
  }
}
/*
 * shutdown task scheduler; makes sure all underlying threads are stopped
 */
void ThreadPerTaskScheduler::shutdown() {}

/*
 * notify scheduler that a given task is ready
 */
void ThreadPerTaskScheduler::notifyReady(const std::shared_ptr<Task>& task) {
  // if task was found in wait set, schedule task
  std::thread t((TaskExecutor(task)));
  t.detach();
}
}
}  // namespace hyrise::taskscheduler
