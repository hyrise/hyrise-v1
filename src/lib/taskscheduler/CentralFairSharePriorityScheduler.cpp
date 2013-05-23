/*
 * CentralFairSharePriorityScheduler.cpp
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#include "CentralFairSharePriorityScheduler.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<CentralFairSharePriorityScheduler>("CentralFairSharePriorityScheduler");
}

void CentralFairSharePriorityScheduler::schedule(std::shared_ptr<Task> task){
  // if task arrives with highest priority, schedule -> this is currently used for RequestParseTask, as we do not have a session ID
  if(task->getPriority() != Task::HIGH_PRIORITY){
    int session = task->getSessionId();
    // check if session is already present
    auto ret = _sessionMap.find(session);
    if(ret == _sessionMap.end()){
      // check if max number of session reached / this should be moved out of the scheduler at some point in time
      if(_sessions >= MAX_SESSIONS){
        fprintf(stderr, "Max nr of sessions reached - task not scheduled!!!!\n");
      }
      // if not add session
      addSession(session, task->getPriority());
      ret = _sessionMap.find(session);
    }
    // set dynamic priority for task and schedule
    task->setPriority(__sync_fetch_and_add(&_dynPriorities[ret->second],0));
    task->setSessionId(ret->second);
  }
  // get/set dynamic priority
  CentralPriorityScheduler::schedule(task);
}

void CentralFairSharePriorityScheduler::notifyReady(std::shared_ptr<Task> task){
  // check for response task
  auto op =  std::dynamic_pointer_cast<hyrise::access::ResponseTask>(task);
  if(op){
    // calc total duration of query
    int64_t work = calculateTotalWork(op->getPerformanceData());
    // add to total work of session and total work
    auto ret = _workMap.find(task->getSessionId());
    if(ret != _workMap.end()){
      __sync_fetch_and_add(&ret->second, work);
      _totalWork += work;
    }
    else
      fprintf(stderr, "No matching task found in map\n");
    // check if prios need to be updated (TBD - currently after every query)
    // however, only one thread should update at a time
    bool expected = false;
    if(_isUpdatingPrios.compare_exchange_strong(expected,true)){
      updateDynamicPriorities();
      _isUpdatingPrios = false;
    }
  }
  CentralPriorityScheduler::notifyReady(task);
}

void CentralFairSharePriorityScheduler::updateDynamicPriorities(){
  std::vector<int64_t> work(_sessions);
  std::vector<std::pair<double, int> > shares(_sessions);
  // sum up total_work according to values read out of ahm; global _totalWork might change, as we do not stop the scheduler
  int64_t total_work = 0;
  // get work for all sessions
  for(int i = 0; i < _sessions; i++){
    int ret = __sync_fetch_and_add(&_workMap.find(i)->second,0);
    work[i] = ret;
    total_work += ret;
  }
  // calculate relative deviation
  double ws, ts;
  for(int i = 0; i < _sessions; i++){
    total_work == 0 ? ws = 0 : ws = (double)work[i]/total_work;
    ts = (double)_extPriorities[i]/_totalPriorities;
    shares[i] = std::make_pair((ts - ws)/ts, i);
  }
  // sort by share deviation
  std::sort(shares.begin(), shares.end());
  // assign priorities
  for(int i = 0; i < _sessions; i++){
    // atomically set the dynamic priority according to the sort order
    __sync_lock_test_and_set(&_dynPriorities[shares[i].second],_sessions + Task::HIGH_PRIORITY - i);
  }

  // TBD: reset work at some point of time - currently not done for testing purposes

/*
  std::cout << "updateDynmicPriorities -> print statistics" << std::endl;
  std::cout << "\t_workMap  -  total work:" << total_work << std::endl;
  for(int i = 0; i < _sessions; i++){
    std::cout << "\t\tsession: " << _workMap.find(i)->first << " work: " <<   __sync_fetch_and_add(&_workMap.find(i)->second,0) << std::endl;
  }
  std::cout << "\tprios  - total prios:" << _totalPriorities << std::endl;
    for(int i = 0; i < _sessions; i++){
      std::cout << "\t\tsession: " << i << " extPrio: " << _extPriorities[i]<<  " dyn prio: " <<   __sync_fetch_and_add(&_dynPriorities[i],0) <<  std::endl;
    }

    std::cout << "\tshares" << std::endl;
    for(int i = 0; i < _sessions; i++){
      total_work == 0 ? ws = 0 : ws = (double)work[i]/total_work;
      ts = (double)_extPriorities[i]/_totalPriorities;
      std::cout << "\t\tsession: " << i << " ws: " << ws <<  " ts: " << ts << std::endl;
    }
    std::cout << "\tshare devaition" << std::endl;
    for(int i = 0; i < _sessions; i++){
      std::cout << "\t\tsession: " << shares[i].second << " share: " << shares[i].first  << std::endl;
    }

*/
}

void CentralFairSharePriorityScheduler::addSession(int session, int priority){
  // internal session number is session count
  int internal_session_id = _sessions.fetch_add(1);
  _sessionMap.insert(session, internal_session_id);

  // set priority
  _totalPriorities += priority;
  __sync_fetch_and_add( &_extPriorities[internal_session_id], priority);

  // add to workMap with WorkShare equal

  // calculate work according to priority
  double work = (double)(priority/_totalPriorities) *_totalWork;

  // check if slot is already used in map
  auto ret = _workMap.find(internal_session_id);
  if(ret == _workMap.end())
    _workMap.insert(internal_session_id, work);
  else
    ret->second = work;
  _totalWork += work;

  // update Priorities
  updateDynamicPriorities();
}

// delete session tbd
void CentralFairSharePriorityScheduler::deleteSession(int session){
}

int64_t CentralFairSharePriorityScheduler::calculateTotalWork(OutputTask::performance_vector& perf_vector){
  int64_t work = 0;
  for(size_t i = 0, size = perf_vector.size(); i < size; i++){
    work += perf_vector[i]->duration;
  }
  return work;
}
