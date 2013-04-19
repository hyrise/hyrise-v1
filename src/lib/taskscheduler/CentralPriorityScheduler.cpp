/*
 * CentralPriorityScheduler.cpp
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#include "CentralPriorityScheduler.h"

log4cxx::LoggerPtr CentralPriorityScheduler::_logger = log4cxx::Logger::getLogger("taskscheduler.CentralPriorityScheduler");

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<CentralPriorityScheduler>("CentralPriorityScheduler");
}

CentralPriorityScheduler::CentralPriorityScheduler(int threads) {
  // create and launch threads
  for(int i = 0; i < threads; i++)
    _worker_threads.push_back(new std::thread(PriorityWorkerThread(*this)));
}

CentralPriorityScheduler::~CentralPriorityScheduler() {
  // wait until all threads have joined
  if(_worker_threads.size() > 0)
    shutdown();
}

void PriorityWorkerThread::operator()(){
  //infinite thread loop
  while (1) {
    //block protected by _threadStatusMutex
    {
      std::lock_guard<std::mutex> lk1(scheduler._statusMutex);
      if (scheduler._status == scheduler.TO_STOP)
        break;
    }
    // lock queue to get task
    std::unique_lock<std::mutex> ul(scheduler._queueMutex);
    // get task and execute
    if (scheduler._runQueue.size() > 0) {
      std::shared_ptr<Task> task = scheduler._runQueue.top();
      // get first task
      scheduler._runQueue.pop();
      ul.unlock();
      if (task) {
        (*task)();
        LOG4CXX_DEBUG(scheduler._logger, "Executed task " << task->vname() << "; hex " << std::hex << &task << std::dec);
        // notify done observers that task is done
        task->notifyDoneObservers();
      }
    }
    // no task in runQueue -> sleep and wait for new tasks
    else {
      //if queue still empty go to sleep and wait until new tasks have been arrived
      if (scheduler._runQueue.size() < 1) {
        // if thread is about to stop, break execution loop
        {
          std::lock_guard<std::mutex> lk1(scheduler._statusMutex);
          if (scheduler._status != scheduler.RUN)
            continue;
        }
        scheduler._condition.wait(ul);
      }
    }
  }
}

/*
 * schedule a task for execution
 */
void CentralPriorityScheduler::schedule(std::shared_ptr<Task> task){
  // simple strategy: check if task is ready to run -> push to run_queue
  // otherwise store in wait list

  // lock the task - otherwise, a notify might happen prior to the task being added to the wait set
  task->lockForNotifications();
  if (task->isReady()){
    std::lock_guard<std::mutex> lk(_queueMutex);
    _runQueue.push(task);
    _condition.notify_one();
  }
  else {
    task->addReadyObserver(this);
    std::lock_guard<std::mutex> lk(_setMutex);
    _waitSet.insert(task);
    LOG4CXX_DEBUG(_logger,  "Task " << std::hex << (void *)task.get() << std::dec << " inserted in wait queue");
  }
  task->unlockForNotifications();
}
/*
 * shutdown task scheduler; makes sure all underlying threads are stopped
 */
void CentralPriorityScheduler::shutdown(){
  _statusMutex.lock();
  _status = TO_STOP;
  _statusMutex.unlock();
  _condition.notify_all();
  for(size_t i = 0; i < _worker_threads.size(); i++)
    _worker_threads[i]->join();
  _worker_threads.clear();
}
/*
 * resize the number of worker threads/queues
 */
void CentralPriorityScheduler::resize(const size_t threads){
  // set status to RESIZING
  // lock scheduler
  _statusMutex.lock();
  _status = RESIZING;
  _statusMutex.unlock();
  if (threads > _worker_threads.size()) {
    size_t addiditional_threads = threads - _worker_threads.size();
    for(size_t i = 0; i < addiditional_threads; i++)
      _worker_threads.push_back(new std::thread(PriorityWorkerThread(*this)));
  } else if (threads < _worker_threads.size()) {
    // hack: stop all, start new, otherwise complicated to stop selected threads
    shutdown();
    for(size_t i = 0; i < threads; i++)
      _worker_threads.push_back(new std::thread(PriorityWorkerThread(*this)));

  }
  _statusMutex.lock();
  _status = RUN;
  _statusMutex.unlock();
}
/**
 * get number of worker
 */
size_t CentralPriorityScheduler::getNumberOfWorker() const{
  return _worker_threads.size();
}

/*
 * notify scheduler that a given task is ready
 */
void CentralPriorityScheduler::notifyReady(std::shared_ptr<Task> task) {
  // remove task from wait set
  _setMutex.lock();
  int tmp = _waitSet.erase(task);
  _setMutex.unlock();

  // if task was found in wait set, schedule task to next queue
  if (tmp == 1) {
    LOG4CXX_DEBUG(_logger, "Task " << std::hex << (void *)task.get() << std::dec << " ready to run");
    std::lock_guard<std::mutex> lk(_queueMutex);
    _runQueue.push(task);
    _condition.notify_one();
  } else
    // should never happen, but check to identify potential race conditions
    LOG4CXX_ERROR(_logger, "Task that notified to be ready to run was not found / found more than once in waitSet! " << std::to_string(tmp));
}
