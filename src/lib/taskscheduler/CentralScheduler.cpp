/*
 * CentralScheduler.cpp
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#include "CentralScheduler.h"

log4cxx::LoggerPtr CentralScheduler::_logger = log4cxx::Logger::getLogger("taskscheduler.CentralScheduler");

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<CentralScheduler>("CentralScheduler");
}

CentralScheduler::CentralScheduler(int threads) {
  // create and launch threads
  for(int i = 0; i < threads; i++)
    _worker_threads.push_back(new std::thread(WorkerThread(*this)));
}

CentralScheduler::~CentralScheduler() {
  // wait until all threads have joined
  if(_worker_threads.size() > 0)
    shutdown();
}

void WorkerThread::operator()(){
  //infinite thread loop
  while (1) {
    if (scheduler._status == scheduler.TO_STOP){
      break;
    }
    // lock queue to get task
    std::unique_lock<std::mutex> ul(scheduler._queueMutex);
    // get task and execute
    if (scheduler._runQueue.size() > 0) {
      std::shared_ptr<Task> task = scheduler._runQueue.front();
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

        if (scheduler._status != scheduler.RUN)
          continue;

        scheduler._condition.wait(ul);
      }
    }
  }
}

/*
 * schedule a task for execution
 */
void CentralScheduler::schedule(std::shared_ptr<Task> task){
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
void CentralScheduler::shutdown(){
  {
    std::lock_guard<std::mutex> lk(_queueMutex);
    {
      _status = TO_STOP;
    }
    //wake up thread in case thread is sleeping
    _condition.notify_all();
  }
  for(size_t i = 0; i < _worker_threads.size(); i++){
    _worker_threads[i]->join();
  }
  _worker_threads.clear();
}
/*
 * resize the number of worker threads/queues
 */
void CentralScheduler::resize(const size_t threads){
  // set status to RESIZING
  _status = RESIZING;
  if (threads > _worker_threads.size()) {
    std::lock_guard<std::mutex> lk(_queueMutex);
    size_t addiditional_threads = threads - _worker_threads.size();
    for(size_t i = 0; i < addiditional_threads; i++)
      _worker_threads.push_back(new std::thread(WorkerThread(*this)));
  } else if (threads < _worker_threads.size()) {
    // hack: stop all, start new, otherwise complicated to stop selected threads
    shutdown();
    std::lock_guard<std::mutex> lk(_queueMutex);
    for(size_t i = 0; i < threads; i++)
      _worker_threads.push_back(new std::thread(WorkerThread(*this)));
  }
  _status = RUN;
}
/**
 * get number of worker
 */
size_t CentralScheduler::getNumberOfWorker() const{
  return _worker_threads.size();
}

/*
 * notify scheduler that a given task is ready
 */
void CentralScheduler::notifyReady(std::shared_ptr<Task> task) {
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
