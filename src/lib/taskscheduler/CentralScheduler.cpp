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

CentralScheduler::CentralScheduler(int threads): _threads(threads) {}

void CentralScheduler::init(){
  _status = START_UP;
  int core = 0;
  int NUM_PROCS = getNumberOfCoresOnSystem();
  // bind threads to cores                                                                                                                                        
  for(int i = 0; i < _threads; i++){
    std::thread thread(WorkerThread(*this));
    hwloc_cpuset_t cpuset;
    hwloc_obj_t obj;
    hwloc_topology_t topology = getHWTopology();
    
    core = (core % (NUM_PROCS - 1)) + 1;
    
    obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, core);
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
  _status = RUN;
}

CentralScheduler::~CentralScheduler() {
  // wait until all threads have joined
  if(_worker_threads.size() > 0)
    shutdown();
}

void WorkerThread::operator()(){
  // worker loop
  while (true) {
    if (scheduler._status == scheduler.TO_STOP) { break; } // break out when asked
    std::shared_ptr<Task> task = nullptr;
    if (scheduler._runQueue.try_pop(task)) {
      (*task)();
      task->notifyDoneObservers();
    } else {
      std::this_thread::yield();
    }
  }
}

/*
 * schedule a task for execution
 */
void CentralScheduler::schedule(std::shared_ptr<Task> task){
  task->lockForNotifications();
  if (task->isReady()) {
    _runQueue.push(task);
  } else {
    task->addReadyObserver(shared_from_this());
  }
  task->unlockForNotifications();
}

/*
 * shutdown task scheduler; makes sure all underlying threads are stopped
 */
void CentralScheduler::shutdown(){
  {
    _status = TO_STOP;
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
  _runQueue.push(task);
}

} } // namespace hyrise::taskscheduler

