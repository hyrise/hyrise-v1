/*
 * CentralFairSharePriorityScheduler.cpp
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#include "CentralFairSharePriorityScheduler.h"
#include "helper/epoch.h"

// register Scheduler at SharedScheduler
namespace {
bool registered  =
    SharedScheduler::registerScheduler<CentralFairSharePriorityScheduler>("CentralFairSharePriorityScheduler");
}

void CentralFairSharePriorityScheduler::schedule(std::shared_ptr<Task> task){
  // if task arrives with highest priority, schedule -> this is currently used for RequestParseTask, as we do not have a session ID prior to parsing the JSON
  if(task->isReady() && (task->getPriority() != Task::HIGH_PRIORITY)){
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

    //update activity
    std::lock_guard<std::mutex> lk(_activityMutex);
    _userActivity[ret->second] = get_epoch_nanoseconds();
  }

  // addDoneObserver to get task execution time
  task->addDoneObserver(this);

  CentralPriorityScheduler::schedule(task);
}

void CentralFairSharePriorityScheduler::notifyReady(std::shared_ptr<Task> task) {
  // if task arrives with highest priority, schedule -> this is currently used for RequestParseTask, as we do not have a session ID
  if(task->isReady() && (task->getPriority() != Task::HIGH_PRIORITY)){
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

  CentralPriorityScheduler::notifyReady(task);
}

void CentralFairSharePriorityScheduler::notifyDone(std::shared_ptr<Task> task){
  {
    //update activity
    std::lock_guard<std::mutex> lk(_activityMutex);
    if(task->getSessionId()!= Task::SESSION_ID_NOT_SET)
      _userActivity[task->getSessionId()] = get_epoch_nanoseconds();
  }
  /*

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
     epoch_t currentTime = get_epoch_nanoseconds();
     std::cout << " op finished: " << currentTime << " vs " << _lastUpdatePrios <<  " delta " << currentTime - _lastUpdatePrios << std::endl;
     if(currentTime - _lastUpdatePrios >= PRIO_UPDATE_INTERVALL){
       _lastUpdatePrios = currentTime;
       if(_isUpdatingPrios.compare_exchange_strong(expected,true)){
         updateDynamicPriorities();
         _isUpdatingPrios = false;
       }
     }
   }
   */
  auto op =  std::dynamic_pointer_cast<OutputTask>(task);
   if(op){
     // calc total duration of query; therefore check first, if performance data has been set
     int64_t work = ! &op->getPerformanceData() ? 0 : op->getPerformanceData().data;
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
     epoch_t currentTime = get_epoch_nanoseconds();
     //std::cout << " op finished: " << currentTime << " vs " << _lastUpdatePrios <<  " delta " << currentTime - _lastUpdatePrios << std::endl;
     if(currentTime - _lastUpdatePrios >= PRIO_UPDATE_INTERVALL){
       _lastUpdatePrios = currentTime;
       if(_isUpdatingPrios.compare_exchange_strong(expected,true)){
         updateDynamicPriorities();
         _isUpdatingPrios = false;
       }
     }
   }
}

void CentralFairSharePriorityScheduler::updateDynamicPriorities(){
  std::vector<int64_t> work(_sessions);
  std::vector<std::pair<double, int> > shares(_sessions);
  // sum up total_work according to values read out of ahm; global _totalWork might change, as we do not stop the scheduler
  int64_t total_work = 0;
  // get work for all sessions
  for(int i = 0; i < _sessions; i++){
    // atomically fetch work and set to zero by AND with 0
    int ret = __sync_fetch_and_and(&_workMap.find(i)->second,0);
    work[i] = ret;
    total_work += ret;
  }
  // calculate relative deviation
  double ws, ts;
  bool isUserActive = true;
  int activeUsers = 0;
  for(int i = 0; i < _sessions; i++){
    // check if user is active
    std::lock_guard<std::mutex> lk(_activityMutex);
    // if activity happened after last update or INACTIVE_USER_INTERVALL seconds before, user is considered as active
    isUserActive = (_lastUpdatePrios < _userActivity[i]) || ((_lastUpdatePrios - _userActivity[i]) < (epoch_t)(INACTIVE_USER_INTERVALL * 1000000000))  ? true : false;
    if(isUserActive) activeUsers ++;

    //std::cout << " session " << i << " is active? " << isUserActive << " " << _lastUpdatePrios - _userActivity[i] <<  " " << _lastUpdatePrios << " " << _userActivity[i]  << " activeUsers " << activeUsers << std::endl;
  }
  for(int i = 0; i < _sessions; i++){
    // check if user is active and not he only user on the system
    if(isUserActive && activeUsers > 1){
      // calculate workshare of last intervall
      ws =  total_work == 0 ? 0 : (double)work[i]/total_work;
      // calculate smoothed workshare
      ts = (double)_extPriorities[i]/_totalPriorities;
      /*
      double newSmoothedWorkShares;
      newSmoothedWorkShares = _smoothedWorkShares[i] == 0 ? ws : (double)ws*DATA_SMOOTHING_FACTOR + (double)(1-DATA_SMOOTHING_FACTOR)*(_smoothedWorkShares[i]+_trendWorkShares[i]);
      _trendWorkShares[i] = _smoothedWorkShares[i] == 0 ? 0 : (double)TREND_SMOOTHING_FACTOR*(newSmoothedWorkShares- _smoothedWorkShares[i]) + (double)(1-TREND_SMOOTHING_FACTOR)*_trendWorkShares[i];
      _smoothedWorkShares[i] = newSmoothedWorkShares;

      shares[i] = std::make_pair((ts - _smoothedWorkShares[i]), i);
*/
      shares[i] = std::make_pair((ts - _averageWorkShares[i].add(ws))/ts, i);
    }
    else{
      ts = (double)_extPriorities[i]/_totalPriorities;
      ws = ts;
      //_smoothedWorkShares[i] = ws;
      _averageWorkShares[i].add(ws);
      shares[i] = std::make_pair(0, i);
    }

  }
  // sort by share deviation
  std::sort(shares.begin(), shares.end());
  if(total_work != 0){
    // assign priorities
    for(int i = 0; i < _sessions; i++){
      // atomically set the dynamic priority according to the sort order
      __sync_lock_test_and_set(&_dynPriorities[shares[i].second],_sessions + Task::HIGH_PRIORITY - i);
    }
  }
  else{
    for(int i = 0; i < _sessions; i++){
      // atomically set the dynamic priority according to the sort order
      __sync_lock_test_and_set(&_dynPriorities[shares[i].second],Task::HIGH_PRIORITY + 1);
    }
  }

  _epoch++;
/*


  std::cout << "updateDynmicPriorities -> print statistics" << std::endl;
  std::cout << "active users " << activeUsers << std::endl;
  std::cout << "\t_workMap  -  total work:" << total_work << std::endl;
  for(int i = 0; i < _sessions; i++){
    if(total_work > 0)
      std::cout << "\t\tsession: " << _workMap.find(i)->first << " work: " <<  work[i] << " work share in interval " << (double)work[i]/total_work<< std::endl;
  }
  std::cout << "\tprios  - total prios:" << _totalPriorities << std::endl;
  for(int i = 0; i < _sessions; i++){
    std::cout << "\t\tsession: " << i << " extPrio: " << _extPriorities[i]<<  " dyn prio: " <<   __sync_fetch_and_add(&_dynPriorities[i],0) <<  std::endl;
  }

  std::cout << "\tshares" << std::endl;
  for(int i = 0; i < _sessions; i++){
    //total_work == 0 ? ws = 0 : ws = (double)work[i]/total_work;
    ts = (double)_extPriorities[i]/_totalPriorities;
    std::cout << "\t\tsession: " << i << " ws: " << _averageWorkShares[i].getAverage() <<  " ts: " << ts << std::endl;//_smoothedWorkShares[i]
  }
  std::cout << "\tshare devaition" << std::endl;
  for(int i = 0; i < _sessions; i++){
    std::cout << "\t\tsession: " << shares[i].second << " share: " << shares[i].first  << std::endl;
  }

  std::cout << "\tuser activity" << std::endl;
  for(int i = 0; i < _sessions; i++){
    std::cout << "\t\tsession: " << shares[i].second << " useractivity: " << _userActivity[i]  << std::endl;
  }
*/
}

void CentralFairSharePriorityScheduler::addSession(int session, int priority){
  std::lock_guard<std::mutex> lk(_addSessionMutex);
  // check if session is not in map
  if(_sessionMap.find(session) == _sessionMap.end()){

    // internal session number is session count
    int internal_session_id = _sessions.fetch_add(1);
    _sessionMap.insert(session, internal_session_id);

    // set priority
    _totalPriorities += priority;
    __sync_fetch_and_add( &_extPriorities[internal_session_id], priority);

    // calculate work according to priority
    double work = (double)(priority/_totalPriorities) *_totalWork;

    // check if slot is already used in map
    auto ret = _workMap.find(internal_session_id);
    if(ret == _workMap.end())
      _workMap.insert(internal_session_id, work);
    else
      ret->second = work;
    _totalWork += work;

    //set work to zero (to ensure equal priorities at start up)
    for(int i = 0; i < _sessions; i++){
      _averageWorkShares[i].reset();
    }

    bool expected = false;
    // update Priorities
    if(_isUpdatingPrios.compare_exchange_strong(expected,true)){
      updateDynamicPriorities();
      _isUpdatingPrios = false;
    }
  }
}

// delete session tbd
void CentralFairSharePriorityScheduler::deleteSession(int session){
  // remove internal session number
  //_sessionMap.find(session);
}

int64_t CentralFairSharePriorityScheduler::calculateTotalWork(OutputTask::performance_vector& perf_vector){
  int64_t work = 0;
  for(size_t i = 0, size = perf_vector.size(); i < size; i++){
    work += perf_vector[i]->duration;
  }
  return work;
}
