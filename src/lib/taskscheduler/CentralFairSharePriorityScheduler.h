/*
 * CentralFairPriorityScheduler.h
 *
 *  Created on: Mar 20, 2013
 *      Author: jwust
 */

#ifndef CENTRALFAIRSHAREPRIORITYSCHEDULER_H_
#define CENTRALFAIRSHAREPRIORITYSCHEDULER_H_

#include "CentralPriorityScheduler.h"
#include "folly/AtomicHashMap.h"
#include "access/ResponseTask.h"
#include "access/OutputTask.h"

using folly::AtomicHashMap;
using std::map;
using std::vector;

/**
 * a CentralFairShare scheduler holds a task queue and n worker threads
 */
class CentralFairSharePriorityScheduler : public CentralPriorityScheduler {
  static const int MAX_SESSIONS = 1000;
  // maps internal session id to work done so far
  AtomicHashMap<int,int64_t> _workMap;
  // vector of dynamically calculated priorities
  vector<int> _dynPriorities;
  // vector of externally provided calculated priorities
  vector<int> _extPriorities;
  // maps external to internal session id (introduced to avoid resizing of _workMap, and to be able to use sessionID as index into Priority vectors)
  AtomicHashMap<int, int> _sessionMap;
  // sum of total work (currently not used)
  std::atomic<int64_t> _totalWork;
  // sum of total priorities of all sessions
  std::atomic<int> _totalPriorities;
  // number of open sessions
  std::atomic<int> _sessions;
  // synchronize updating priorities - only one thread should do this at a time
  std::atomic<bool> _isUpdatingPrios;

  int64_t calculateTotalWork(OutputTask::performance_vector& perf_vector);

public:

  //we allow max 1000 sessions; ahm should not be dynamically resized
  CentralFairSharePriorityScheduler(int threads = getNumberOfCoresOnSystem()): CentralPriorityScheduler(threads),
                                                                      _workMap(MAX_SESSIONS),
                                                                      _dynPriorities(MAX_SESSIONS, 0),
                                                                      _extPriorities(MAX_SESSIONS, 0),
                                                                      _sessionMap(MAX_SESSIONS),
                                                                      _totalWork(0),
                                                                      _totalPriorities(0),
                                                                      _sessions(0),
                                                                      _isUpdatingPrios(false){
  };
  virtual ~CentralFairSharePriorityScheduler(){ };

  void schedule(std::shared_ptr<Task> task);
  virtual void notifyReady(std::shared_ptr<Task> task);
  void updateDynamicPriorities();
  void addSession(int session, int priority);
   //tbd
  void deleteSession(int session);
};

#endif /* CENTRALFAIRSHAREPRIORITYSCHEDULER_H_ */
