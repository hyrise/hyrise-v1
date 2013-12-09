/*
 * CentralScheduler.cpp
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#include "CentralScheduler.h"

namespace hyrise {
namespace taskscheduler {

log4cxx::LoggerPtr CentralScheduler::_logger = log4cxx::Logger::getLogger("taskscheduler.CentralScheduler");

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<CentralScheduler>("CentralScheduler");
}

CentralScheduler::CentralScheduler(int threads) {
    _status = START_UP;
  // create and launch threads
  if(threads > getNumberOfCoresOnSystem()){
    fprintf(stderr, "Tried to use more threads then cores - no binding of threads takes place\n");
    for(int i = 0; i < threads; i++){
      _worker_threads.emplace_back(WorkerThread(*this));
    }
  } else {
    // bind threads to cores
    for(int i = 0; i < threads; i++){
      //_worker_threads.push_back(new std::thread(WorkerThread(*this)));
      std::thread thread(WorkerThread(*this));
      hwloc_cpuset_t cpuset;
      hwloc_obj_t obj;
      hwloc_topology_t topology = getHWTopology();

      obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, i);
      // the bitmap to modify
      cpuset = hwloc_bitmap_dup(obj->cpuset);
      // remove hyperthreads
      hwloc_bitmap_singlify(cpuset);
      // bind
      if (hwloc_set_thread_cpubind(topology, thread.native_handle(), cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND)) {
        char *str;
        int error = errno;
        hwloc_bitmap_asprintf(&str, obj->cpuset);
        fprintf(stderr, "Couldn't bind to cpuset %s: %s\n", str, strerror(error));
        fprintf(stderr, "Continuing as normal, however, no guarantees\n");
        free(str);
      }

      // assuming single machine system                                                                                                         
      obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_MACHINE, 0);
      // set membind policy interleave for this thread                                                                                          
      if (hwloc_set_membind_nodeset(topology, obj->nodeset, HWLOC_MEMBIND_INTERLEAVE, HWLOC_MEMBIND_STRICT | HWLOC_MEMBIND_THREAD)) {
	char *str;
	int error = errno;
	hwloc_bitmap_asprintf(&str, obj->nodeset);
	fprintf(stderr, "Couldn't membind to nodeset  %s: %s\n", str, strerror(error));
	fprintf(stderr, "Continuing as normal, however, no guarantees\n");
	free(str);
      }

      hwloc_bitmap_free(cpuset);
      _worker_threads.push_back(std::move(thread));
    }
  }
  _status = RUN;
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
    std::unique_lock<lock_t> ul(scheduler._queueMutex);
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
    std::lock_guard<lock_t> lk(_queueMutex);
    _runQueue.push(task);
    _condition.notify_one();
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
void CentralScheduler::shutdown(){
  {
    std::lock_guard<lock_t> lk(_queueMutex);
    {
      _status = TO_STOP;
    }
    //wake up thread in case thread is sleeping
    _condition.notify_all();
  }
  for(size_t i = 0; i < _worker_threads.size(); i++){
    _worker_threads[i].join();
  }
  _worker_threads.clear();
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
    std::lock_guard<lock_t> lk(_queueMutex);
    _runQueue.push(task);
    _condition.notify_one();
  } else
    // should never happen, but check to identify potential race conditions
    LOG4CXX_ERROR(_logger, "Task that notified to be ready to run was not found / found more than once in waitSet! " << std::to_string(tmp));
}

} } // namespace hyrise::taskscheduler

