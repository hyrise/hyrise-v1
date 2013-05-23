// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * Task.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: jwust
 */

#include "taskscheduler/Task.h"

#include <iostream>
#include <thread>

void Task::lockForNotifications() {
  _notifyMutex.lock();
}

void Task::unlockForNotifications() {
  _notifyMutex.unlock();
}

void Task::notifyReadyObservers() {
	std::lock_guard<std::mutex> lk(_observerMutex);
	std::vector<TaskReadyObserver *>::iterator itr;
	for (itr = _readyObservers.begin(); itr != _readyObservers.end(); ++itr) {
		(*itr)->notifyReady(shared_from_this());
	}
}

void Task::notifyDoneObservers() {
	std::lock_guard<std::mutex> lk(_observerMutex);
	std::vector<TaskDoneObserver *>::iterator itr;
	for (itr = _doneObservers.begin(); itr != _doneObservers.end(); ++itr) {
		(*itr)->notifyDone(shared_from_this());
	}
}

Task::Task(): _dependencyWaitCount(0), _preferredCore(NO_PREFERRED_CORE), _preferredNode(NO_PREFERRED_NODE), _priority(DEFAULT_PRIORITY), _sessionId(0), _id(0) {
}

void Task::addDependency(std::shared_ptr<Task> dependency) {
  {
    std::lock_guard<std::mutex> lk(_depMutex);
    _dependencies.push_back(dependency);
    ++_dependencyWaitCount;
  }
  dependency->addDoneObserver(this);

}

void Task::addReadyObserver(TaskReadyObserver *observer) {

  std::lock_guard<std::mutex> lk(_observerMutex);
  _readyObservers.push_back(observer);
}

void Task::addDoneObserver(TaskDoneObserver *observer) {

  std::lock_guard<std::mutex> lk(_observerMutex);
  _doneObservers.push_back(observer);
}

void Task::notifyDone(std::shared_ptr<Task> task) {
  _depMutex.lock();
  int t = --_dependencyWaitCount;
  _depMutex.unlock();

  if (t == 0) {
    if(_preferredCore == NO_PREFERRED_CORE && _preferredNode == NO_PREFERRED_NODE)
      _preferredNode = task->getActualNode();
    std::lock_guard<std::mutex> lk(_notifyMutex);
    notifyReadyObservers();
  }
}

bool Task::isReady() {
  std::lock_guard<std::mutex> lk(_depMutex);
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
    std::lock_guard<std::mutex> lock(_mut);
    _finished = true;
  }
  _cond.notify_one();
}

void WaitTask::wait() {
  std::unique_lock<std::mutex> ul(_mut);
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
