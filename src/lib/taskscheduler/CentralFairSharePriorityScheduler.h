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
#include "access/system/ResponseTask.h"
#include "access/system/OutputTask.h"
#include <mutex>
#include "helper/RollingAverage.h"

using folly::AtomicHashMap;
using std::map;
using std::vector;

/**
 * a CentralFairShare scheduler holds a task queue and n worker threads
 */
class CentralFairSharePriorityScheduler : public CentralPriorityScheduler, TaskDoneObserver {
  static const int MAX_SESSIONS = 1000;
  //static const int AVERAGE_SAMPLE_SIZE = 100;
  static const uint64_t PRIO_UPDATE_INTERVALL = 200000000;
  static const int INACTIVE_USER_INTERVALL = 20;
  // weighs importance of last interval
  //static constexpr double DATA_SMOOTHING_FACTOR = 0.1;
  //static constexpr double TREND_SMOOTHING_FACTOR = 0.0;
  // maps internal session id to work done so far
  AtomicHashMap<int,int64_t> _workMap;
  // vector of dynamically calculated priorities
  vector<int> _dynPriorities;
  // vector of externally provided calculated priorities
  vector<int> _extPriorities;
  // vector of exponentially smoothed work_shares
  //vector<double> _smoothedWorkShares;
  //vector<double> _trendWorkShares;
  vector<epoch_t> _userActivity;
  vector<int> _isOnlyUser;
  std::mutex _activityMutex;
  // rolling average
  vector<RollingAverage<float> > _averageWorkShares;
  // maps external to internal session id (introduced to avoid resizing of _workMap, and to be able to use sessionID as index into Priority vectors)
  AtomicHashMap<int, int> _sessionMap;
  // sum of total work
  std::atomic<int64_t> _totalWork;
  // sum of total priorities of all sessions
  std::atomic<int> _totalPriorities;
  // number of open sessions
  std::atomic<int> _sessions;
  // synchronize updating priorities - only one thread should do this at a time
  std::atomic<bool> _isUpdatingPrios;
  // timepoint of last update
  std::atomic<epoch_t> _lastUpdatePrios;
  // mutex to avoid duplicate sessions
  std::mutex _addSessionMutex;
  std::atomic<int> _epoch;

  int64_t calculateTotalWork(OutputTask::performance_vector& perf_vector);

public:

  //we allow max 1000 sessions; ahm should not be dynamically resized
  CentralFairSharePriorityScheduler(int threads = getNumberOfCoresOnSystem()): CentralPriorityScheduler(threads),
                                                                      _workMap(MAX_SESSIONS),
                                                                      _dynPriorities(MAX_SESSIONS, 0),
                                                                      _extPriorities(MAX_SESSIONS, 0),
                                                                      //_smoothedWorkShares(MAX_SESSIONS,0),
                                                                      //_trendWorkShares(MAX_SESSIONS,0),
                                                                      _userActivity(MAX_SESSIONS,0),
                                                                      _isOnlyUser(MAX_SESSIONS, 0),
                                                                      _averageWorkShares(MAX_SESSIONS, RollingAverage<float>(50)),
                                                                      _sessionMap(MAX_SESSIONS),
                                                                      _totalWork(0),
                                                                      _totalPriorities(0),
                                                                      _sessions(0),
                                                                      _isUpdatingPrios(false),
                                                                      _lastUpdatePrios(0),
                                                                      _epoch(0){
  };
  virtual ~CentralFairSharePriorityScheduler(){ };
  void schedule(std::shared_ptr<Task> task);
  virtual void notifyDone(std::shared_ptr<Task> task);
  virtual void notifyReady(std::shared_ptr<Task> task);
  void updateDynamicPriorities();
  void addSession(int session, int priority);
   //tbd
  void deleteSession(int session);
};

#endif /* CENTRALFAIRSHAREPRIORITYSCHEDULER_H_ */
