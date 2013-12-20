// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * Task.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#include "taskscheduler/Task.h"

#include "log4cxx/logger.h"

#include <iostream>
#include <thread>
#include <algorithm>

namespace {
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.taskscheduler"));
}

namespace hyrise {
namespace taskscheduler {

std::vector<std::shared_ptr<Task>> Task::applyDynamicParallelization(size_t dynamicCount){
  LOG4CXX_ERROR(_logger, "Dynamic Parallelization has not been implemented for this operator.");
  LOG4CXX_ERROR(_logger, "Running without parallelization.");
  return { shared_from_this() };
}

void Task::lockForNotifications() {
  _notifyMutex.lock();
}

void Task::unlockForNotifications() {
  _notifyMutex.unlock();
}

void Task::notifyReadyObservers() {
  // Lock and copy observers.
  // This way we do not run any callbacks while holding a lock.
  std::vector<std::weak_ptr<TaskReadyObserver>> targets;
	{
    std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
    targets = _readyObservers;
  }
	for (const auto& target : targets) {
    if (auto observer = target.lock()) {
      observer->notifyReady(shared_from_this());  
    }
	}
}

void Task::notifyDoneObservers() {
  // Lock and copy observers.
  // This way we do not run any callbacks while holding a lock.
  std::vector<std::weak_ptr<TaskDoneObserver>> targets;
  {
    std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
    targets = _doneObservers;  
  }
	for (const auto& target : targets) {
    if (auto observer = target.lock()) {
      observer->notifyDone(shared_from_this());  
    }
	}
}

Task::Task(): _dependencyWaitCount(0), _preferredCore(NO_PREFERRED_CORE), _preferredNode(NO_PREFERRED_NODE), _priority(DEFAULT_PRIORITY), _sessionId(SESSION_ID_NOT_SET), _id(0) {
}

void Task::addDependency(std::shared_ptr<Task> dependency) {
  {
    std::lock_guard<decltype(_depMutex)> lk(_depMutex);
    _dependencies.push_back(dependency);
    ++_dependencyWaitCount;
  }
  dependency->addDoneObserver(shared_from_this());
}

void Task::addDoneDependency(std::shared_ptr<Task> dependency) {
  {
    std::lock_guard<decltype(_depMutex)> lk(_depMutex);
    _dependencies.push_back(dependency);
  }
}

void Task::removeDependency(std::shared_ptr<Task> dependency) {
    
    std::lock_guard<decltype(_depMutex)> lk(_depMutex);
    // remove from dependencies
    auto newEnd = std::remove(_dependencies.begin(), _dependencies.end(), dependency);
    if (newEnd != _dependencies.end()) { // we actually removed something
      _dependencies.erase(newEnd, _dependencies.end());
      --_dependencyWaitCount;
    }
}

void Task::changeDependency(std::shared_ptr<Task> from, std::shared_ptr<Task> to) {
    
    std::lock_guard<decltype(_depMutex)> lk(_depMutex);
    // find from dependencies
    for(size_t i = 0, size = _dependencies.size(); i < size; i++){
      if(_dependencies[i] == from){
        _dependencies[i] = to;
      }
    }
    // add new done observer
    to->addDoneObserver(std::dynamic_pointer_cast<Task>(shared_from_this()));
}

void Task::setDependencies(std::vector<std::shared_ptr<Task> > dependencies, int count) {
    std::lock_guard<decltype(_depMutex)> lk(_depMutex);
    _dependencies = dependencies;
    _dependencyWaitCount = 0;
}

bool Task::isDependency(const task_ptr_t& task) {
  std::lock_guard<decltype(_depMutex)> lk(_depMutex);
  return std::any_of(
      _dependencies.begin(),
      _dependencies.end(),
      [task](task_ptr_t t) {return t == task;});
}

void Task::addReadyObserver(const std::shared_ptr<TaskReadyObserver>& observer) {

  std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
  _readyObservers.push_back(observer);
}

void Task::addDoneObserver(const std::shared_ptr<TaskDoneObserver>& observer) {

  std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
  _doneObservers.push_back(observer);
}

void Task::notifyDone(std::shared_ptr<Task> task) {
  _depMutex.lock();
  int t = --_dependencyWaitCount;
  _depMutex.unlock();

  if (t == 0) {
    if(_preferredCore == NO_PREFERRED_CORE && _preferredNode == NO_PREFERRED_NODE)
      _preferredNode = task->getActualNode();
    std::lock_guard<decltype(_notifyMutex)> lk(_notifyMutex);
    notifyReadyObservers();
  }
}

bool Task::isReady() {
  std::lock_guard<decltype(_depMutex)> lk(_depMutex);
  return (_dependencyWaitCount == 0);
}

int Task::getDependencyCount() {
  return _dependencies.size();
}

// TODO make nicer; method needed to identify result task of a query
// in the query tree, we have no successor if we have no doneObserver
bool Task::hasSuccessors() {
  return (_doneObservers.size() > 0);
}

void Task::setPreferredCore(int core) {
  _preferredCore = core;
}

int Task::getPreferredCore() {
  return _preferredCore;
}

WaitTask::WaitTask() {
  _finished = false;
}

void WaitTask::operator()() {
  {
    std::lock_guard<decltype(_mut)> lock(_mut);
    _finished = true;
  }
  _cond.notify_one();
}

void WaitTask::wait() {
  std::unique_lock<decltype(_mut)> ul(_mut);
  while (!_finished) {
    _cond.wait(ul);
  }
}

SleepTask::SleepTask(int microseconds) {
  _microseconds = microseconds;
}

void SleepTask::operator()() {
  std::this_thread::sleep_for(std::chrono::microseconds(_microseconds));
}

void SyncTask::operator()() {
  //do nothing
}

} } // namespace hyrise::taskscheduler

